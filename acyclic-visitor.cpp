//
//			The Acyclic Visitor Pattern
//
//	The Acyclic Visitor Pattern is a varient of the Visitor
//	that allows new visitable classes to be added without,
//	y'know, breaking that junk.
//
//	Each visitable object is paired with a visitor specialized
//	in visiting that object. When the visitable object accepts
//	the visitor, it has to check if the visitor is of the type
//	that visits it. God, that was an awful sentence.
//
//	Use the Acyclic Visitor when you want to be able to add
//	new types of objects frequently and want the rest of the
//	system to be isolated from those changes. It's well suited
//	to cases where a Visitor might only want to visit a subset
//	of the classes.
//	Also, when you want to impress your friends.
//
//	In my component-based game, I use the Acyclic Visitor
//	as a messenger to tell different components messages as needed.
//	For example, the ItemAvailableForPickupMessenger looks
//	through the collection of Components in an Entity to find the
//	PlayerIntent and tell it that an item can be picked up
//	by the player.
//
//	Uh, that last paragraph probs didn't make sense, but trust me,
//	it's rad.

#include <iostream>
#include <vector>
#include <string>
using namespace std;

#define PI 3.1415

class Visitor;

// The purely abstract AbstractVisitor.
// Notice that, much like yours truly,
// this class doesn't actually do anything.
// It's just there so the Shape can accept it.
class AbstractVisitor {
public:
	// The class needs at least one virtual method.
	// (it's a C++ thing.)
	virtual ~AbstractVisitor() {}
};

// Once again, the base Shape class. 
class Shape {
	public:
		virtual ~Shape() {}
		float x, y;
		Shape( float x, float y ) : x(x), y(y) {}

		// The virtual accept method.
		// Note that, with the acyclic visitor,
		// this can actually be implemented in the base.
		// This means that the derived classes can even
		// get away with not implementing the method at all.
		// If you've got a class no one wants to visit,
		// this is a practical way to avoid writing the accept.
		// (Also, ouch, man, that's cold)
		virtual void accept( AbstractVisitor* av ) { }
};

class Rectangle;
class Circle;
class Triangle;

// This is a visitor specialized
// to visit the Recangle.
// Visitors that want to visit the Rectangle
// will derive from this puppy.
// Keep in mind that one of these specialized
// visitors needs to written for every visitable
// object.
class RectangleVisitor {
public:
	virtual void visit( Rectangle* rectangle ) = 0;
};

// The derived Rectangle class.
class Rectangle : public Shape {
public:
	float w, h;
	Rectangle( float x, float y, float w, float h ) : Shape(x,y), w(w), h(h) {}

	// And here is the implementation of the Rectangle's accept.
	// Once again, this needs to be implemented in the derived class.
	// However, it's a little more advanced than the regualr Visitor.
	void accept( AbstractVisitor* av ) {

		// The Rectangle must first check to see if
		// the Visitor is, in fact, a RectangleVisitor.
		// This is done with a dynamic_cast here,
		// but it could conceivably be done with a
		// home-brewed type system for better performance.
		RectangleVisitor* rv = dynamic_cast<RectangleVisitor*>( av );

		// If the visitor is a RectangleVisitor,
		// visit away!
		if ( rv ) {
			rv->visit( this );
		}
	}
};

// Ditto the CircleVisitor.
class CircleVisitor {
public:
	virtual void visit( Circle* circle ) = 0;
};

// And the Circle.
class Circle : public Shape {
public:
	float r;
	Circle( float x, float y, float r ) : Shape(x,y), r(r) {}

	void accept( AbstractVisitor* av ) {

		CircleVisitor* cv = dynamic_cast<CircleVisitor*>( av );
		if ( cv ) {
			cv->visit( this );
		}
	}
};

// And hey, even the TriangleVisitor.
// Note that even without the TriangleVisitor,
// the Triangle class could be created and used
// without breaking jack all.
// However, for the Triangle to be visited,
// we need a TriangleVisitor.
class TriangleVisitor {
public:
	virtual void visit( Triangle* triangle ) = 0;
};

// Errybody's favourite shape!
class Triangle : public Shape {
public:
	float b, h;
	Triangle( float x, float y, float b, float h ) : Shape(x,y), b(b), h(h) {}

	void accept( AbstractVisitor* av ) {

		TriangleVisitor* tv = dynamic_cast<TriangleVisitor*>( av );
		if ( tv ) {
			tv->visit( this );
		}
	}
};

// An example of a concrete visitor.
// The AreaCalculator derives from
// the AbstractVisitor so that it can be accepted.
// It derives from the Circle-and RectangleVisitors
// so that it can visit them.
// It *doesn't* derive from the TriangleVisitor
// because I can't remember how to calculate
// the area of a triangle (b*h/2?)
class AreaCalculator :
	public AbstractVisitor,
	public RectangleVisitor,
	public CircleVisitor {
public:
	void visit( Rectangle* rectangle ) {

		float area;
		area = rectangle->w * rectangle->h;
		cout << "rectangle area is: " << area << endl;
	}

	void visit( Circle* circle ) {

		float area;
		area = PI * circle->r * circle->r;
		cout << "circle area is: " << area << endl;
	}
};

// Remember, the concrete Visitor
// must derive from every one of the
// specialized visitors it wants to
// implement visiting for. It's kind of a pain,
// but the tradeoff is between automatic,
// inherited visiting and this manual kind.
class Namer :
	public AbstractVisitor,
	public RectangleVisitor,
	public CircleVisitor,
	public TriangleVisitor {
public:
	void visit( Rectangle* rectangle ) {
		cout << "name: " << "rectangle" << endl;
	}

	void visit( Circle* circle ) {
		cout << "name: " << "circle" << endl;
	}

	void visit( Triangle* triangle ) {
		cout << "name: " << "triangle" << endl;
	}
};

// For emphasis, it's worth pointing out
// that the RectangleCounter only needs to
// implement the visiting for the RectangleVisitor.
// It doesn't care about any other classes in the system.
class RectangleCounter :
	public AbstractVisitor,
	public RectangleVisitor {
public:
	float count;
	RectangleCounter() : count(0) {}

	void visit(Rectangle* rectangle) { ++count; }
};



typedef vector<Shape*> ShapeList;

int main() {

	ShapeList list;
	list.push_back( new Rectangle(0,0,10,20) );
	list.push_back( new Rectangle(10,10, 5,3) );
	list.push_back( new Circle( 5,5, 15) );
	list.push_back( new Triangle(2,2, 4, 3.14) );

	// areas
	AreaCalculator *ac = new AreaCalculator();
	for ( int i = 0; i < list.size(); ++i ) {
		list[i]->accept( ac );
	}
	delete ac;

	// name
	Namer *n = new Namer();
	for ( int i = 0; i < list.size(); ++i ) {
		list[i]->accept( n );
	}
	delete n;

	// counting
	RectangleCounter *rc = new RectangleCounter();
	for ( int i = 0; i < list.size(); ++i ) {
		list[i]->accept( rc );
	}
	cout << "there are " << rc->count << " rectangles" << endl;
	delete rc;

	for ( int i = 0; i < list.size(); ++i )
		delete list[i];

	return 0;
}
