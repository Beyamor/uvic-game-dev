# bunch of stuff to set up lwjgl with jruby
$:.push File.expand_path("../lib", __FILE__)

require "java"
require "lwjgl.jar"
require "slick.jar"

java_import org.newdawn.slick.BasicGame
java_import org.newdawn.slick.Color
java_import org.newdawn.slick.GameContainer
java_import org.newdawn.slick.Graphics
java_import org.newdawn.slick.Image
java_import org.newdawn.slick.Input
java_import org.newdawn.slick.SlickException
java_import org.newdawn.slick.AppGameContainer

# coupla constants
APP_WIDTH = 640
APP_HEIGHT = 480

# the player ship class
# pretty simple, don't sweat it too much
class Ship

	attr_reader :x, :y

	def initialize(view)
		@view = view
		@sprite = Image.new "resources/ship.png"
		@x = APP_WIDTH/2
		@y = APP_HEIGHT/2
	end

	def handle_input(input)
		@x += 5 if input.is_key_down(Input::KEY_D)
		@x -= 5 if input.is_key_down(Input::KEY_A)
		@y -= 5 if input.is_key_down(Input::KEY_W)
		@y += 5 if input.is_key_down(Input::KEY_S)
	end

	def render()
		# notice here that the Ship is telling the View to draw an image at the Ship's position
		# the View takes that position, which is in world space, and converts it into screen space
		# so, the View that follows the Ship takes the Ship's position in world space and converts it to the center of the screen
		@view.draw_image(@sprite, @x, @y)
	end
end

# okay. we're getting a little more interesting
# the Tiler, well, tiles
# that is to say, fills a view with a sprite,
# tiling (repeating) it horizontally/vertically to completely fill the space
# it's actually a little more general than we need here, but whatevs
class Tiler
	def initialize(view, imageName, h_start, v_start, is_tiling_h, is_tiling_v)
		@view = view
		@tile = Image.new imageName
		@x_offset = h_start
		@y_offset = v_start
		@is_tiling_h = is_tiling_h
		@is_tiling_v = is_tiling_v
	end

	def render()
		# okay. so we're going to be working with the tile's dimensions, right?
		tile_width = @tile.get_width
		tile_height = @tile.get_height

		# dx and dy represent the difference between the top-left corner of the view and where we start drawing the tiles from
		# don't worry too much about this. it's important, but incidental to the main math
		dx = @view.left - @x_offset
		dy = @view.top - @y_offset

		# alright. imagine an infinite grid of tiles expanding in all directions. got it?
		# we need to figure out which tile contains the top-left corner of the view
		# this accomplishes that by, essentially, rounding the coordinates to the nearest tile
		# i like to phrase this as converting from world space (where the view is) to tile space
		min_tile_x = (dx / tile_width).floor
		min_tile_y = (dy / tile_height).floor

		# now that we know which tile contains the top-left corner of the view,
		# we need to find out its position in the world. this is where we'll start drawing it
		# (we're converting from tile space back to world space here)
		min_x = @x_offset + min_tile_x * tile_width
		min_y = @y_offset + min_tile_y * tile_height

		# min_x and min_y represent the smallest coordinates we draw a tile at - ie, the top-left corner of the view
		# max_x and max_y are the largest coordinates we need to draw at to cover the view - the bottom-right corner
		max_x = @view.left + @view.width
		max_y = @view.top + @view.height

		# and that's the worst of the math, now we just build a list of all the places we're going to draw tiles
		# if we're tiling along an axis, this list will be every tile position from min_x/y to max_x/y
		# right? enough to draw tiles across the entire view along that axis
		# if we're not tiling along an axis, we just need to draw at the given offset
		# so, for example, in this game, we tile horizontally, but not vertically
		# that is, the screen gets covered from left to right by tiling, but we just draw at a single y coordinate
		draw_xs = @is_tiling_h? (min_x..max_x).step(tile_width).to_a : [@x_offset]
		draw_ys = @is_tiling_v? (min_y..max_y).step(tile_height).to_a : [@y_offset]

		# phew. okay, go ahead and draw the images into the view
		draw_xs.each do |x|
			draw_ys.each do |y|
				@view.draw_image(@tile, x, y)
			end
		end
	end
end

# here's our actual view class!
# what's a view do?
# the view is responsible for drawing things to the screen
# it's a nice abstraction because you can put the view wherever and let things draw through it
# that means that you can isolate the rest of the system from worrying about how to filter, process, and draw things
# and it's really simple! neat!
class View
	attr_reader :left, :top, :width, :height

	def initialize()
		@left = 0
		@top = 0
		@width = APP_WIDTH
		@height = APP_HEIGHT
	end

	def draw_image(image, x, y)
		# in our game, the translation from world space to screen space is pretty straightforward
		# we just need to subtract the view's current position in world space from that of whatever it's drawing
		# a more complex game might see the view, for example, scaling the coordinates in some way
		# eg, by a pixels per meter ratio
		image.draw(x - @left, y - @top)
	end

	def center(x, y)
		@left = x - @width/2
		@top = y - @height/2
	end
end

# the entityview is a special view that just follows an entity, centering on that entity
class EntityView < View
	attr_writer :entity

	def initialize
		super()
	end

	def update
		center(@entity.x, @entity.y)
	end
end

# the horizontalentityview is like the entityview except that it only follows the entity horizontally
# why? well pal, because
class HorizontalEntityView < View
	attr_writer :entity

	def initialize
		super()
	end

	def update
		center(@entity.x, @height/2)
	end
end

# okay. here's where we get to the heart of it
# a parallaxview is a view which moves relative to another view
# with this, we can create views that move faster or slower than the main view, creating our parallax effect
class ParallaxView < View
	def initialize(view_to_track, relative_speed, do_h_parallax=false, do_v_parallax=false)
		super()
		@tracked_view = view_to_track
		@relative_speed = relative_speed
		@do_h_parallax = do_h_parallax
		@do_v_parallax = do_v_parallax
	end

	def update
		# if the view is moving producing the parallax effect along an axis, it's moving at some relative speed
		# otherwise, it moves at the exactly the same speed as the other view, thus producing the same translation
		# a smaller relative speed (eg, 0.2) will move slower, which is good for, say, the things in the background
		# a larger relative speed (eg, 1.2) will move faster, great for things in the foreground
		@left = @tracked_view.left * (@do_h_parallax? @relative_speed : 1)
		@top = @tracked_view.top * (@do_v_parallax? @relative_speed : 1)
	end
end

class ParallaxGame < BasicGame
	
	def init(container)
		@parallax_views = []
		@renderers = []
		@main_view = HorizontalEntityView.new
		@ship = Ship.new @main_view
		@main_view.entity = @ship

		add_tiler_with_view "resources/Background.png", 0, 0.01
		add_tiler_with_view "resources/Mountains.png", 262, 0.1
		@renderers << Tiler.new(@main_view, "resources/Sand.png", 0, 408, true, false) # don't need parallax for the sand
		add_tiler_with_view "resources/Trees2.png", 356, 0.8
		add_tiler_with_view "resources/Trees.png", 351, 0.9
		@renderers << @ship # let the ship draw above the trees/below the rocks
		add_tiler_with_view "resources/Rocks2.png", 425, 1.1
		add_tiler_with_view "resources/Rocks1.png", 425, 1.2

=begin
		@sand = Tiler.new "resources/Sand.png", 0, 408, true, false
		@rocks = Tiler.new "resources/Rocks1.png", 0, 425, true, false
		@rocks2 = Tiler.new "resources/Rocks2.png", 0, 425, true, false
		@rock_view = ParallaxView.new @main_view, 1.2, true
		@rock_view2 = ParallaxView.new @main_view, 1.1, true
		@trees = Tiler.new "resources/Trees.png", 0, 351, true, false
		@tree_view = ParallaxView.new @main_view, 0.9, true
		@trees2 = Tiler.new "resources/Trees2.png", 0, 356, true, false
		@tree_view2 = ParallaxView.new @main_view, 0.8, true
		@mountains = Tiler.new "resources/Mountains.png", 0, 262, true, false
		@mountain_view = ParallaxView.new @main_view, 0.1, true
		@stars = Tiler.new "resources/Background.png", 0, 0, true, false
		@star_view = ParallaxView.new @main_view, 0.01, true
=end
	end

	# just a helper function to add new tilers and views
	def add_tiler_with_view(tile_image, tiler_y_offset, parallax_speed)
		view = ParallaxView.new @main_view, parallax_speed, true
		tiler = Tiler.new view, tile_image, 0, tiler_y_offset, true, false
	
		@parallax_views << view
		@renderers << tiler
	end

	def handle_input(input)
		container.exit if input.is_key_down(Input::KEY_ESCAPE)
		@ship.handle_input input
	end

	def update(container, delta)
		handle_input container.get_input

		@main_view.update
		@parallax_views.each {|view| view.update}
	end

	def draw_background(graphics)
		graphics.set_color Color.white
		graphics.fill_rect 0, 0, APP_WIDTH, APP_HEIGHT
	end

	def render(container, graphics)
		draw_background graphics
		@renderers.each {|renderer| renderer.render}
	end
end

app = AppGameContainer.new ParallaxGame.new "Parallax"
app.set_display_mode APP_WIDTH, APP_HEIGHT, false
app.set_show_fps false
app.start
