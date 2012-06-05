//
//		The Visitor Pattern
//
//	The Visitor Pattern can be used to separate algorithms
//	from the data it acts on which is a stupid way of saying,
//	it's kind of like adding virtual methods to classes
//	without using virtual methods.
//
//	If you want to get all fancy pants about it,
//	it simulates a double dispatch in languages which don't have it.
//
//	Use the Visitor when you have a collection of objects
//	and want to be able to perform some type-dependent operation
//	on them. One caveat to this, however, is that the
//	class heirarchy for the visitable objects should be stable
//	because adding new ones breaks all kinds of junk.
//
//	For example, a tile on the map in a turn-based strategy game
//	could have a collection of the entities on it. The Visitor
//	could be used to do stuff like make a context menu based
//	on the types of entities on the tile.
//
//	's pretty boss, I guess.
//

#include <iostream>
#include <vector>
#include <string>
using namespace std;

#define PI 3.1415

// A visitor class is forward delcared so we can reference it in the shape class
// (this is one of those "C++ things")
class Visitor;

// The base Shape class.
// This is the "data class."
// It's the classes deriving from Shape that we're really interested in.
class Shape {
	public:
		// Dummy structure to demonstrate the idea of the class.
		virtual ~Shape() {}
		float x, y;
		Shape( float x, float y ) : x(x), y(y) {}

		// The purely virtual accept method.
		// (purely virtual means it must be implemented in the derived class)
		// This method is used by derived Shapes to tell Visitors
		// which class they're visiting.
		virtual void accept( Visitor* visitor ) = 0;
};

// Forward declarations for the classes the Visitor visits.
// Note here that the base Visitor MUST know in advance the Shapes it visits.
class Rectangle;
class Circle;

// The base Visitor class.
// This is the "algorithm" class.
// (Very Interesting Software Implementation To Our Requirements)
class Visitor {
public:
	// Virtual visit methods are created for each of classes the Visitor visits.
	// Note that these visit the derived classes, not the base Shape class.
	// This is because the derived classes tell the Visitor which class
	// it is visiting. Magical type inference hooks that information up to the right method.
	// Implementing a visit( Shape* shape ) would result in that
	// base method being called for every Shape instead of the ones written
	// specifically for the derived classes.
	// Remember, type inferenence magic - it's the bee's knees!
	virtual void visit( Rectangle* rectangle ) = 0;
	virtual void visit( Circle* circle ) = 0;
};

// It's a rectangle!
class Rectangle : public Shape {
public:
	float w, h;
	Rectangle( float x, float y, float w, float h ) : Shape(x,y), w(w), h(h) {}

	// Here, the Rectangle, knowing that it is, in fact, a Rectangle,
	// tells the Visitor to visit, well, a Rectangle.
	// Note that this could be rewritten as visitor->visitRectangle( this )
	// but idiomatically, the method is just called visit for every type.
	void accept( Visitor* visitor ) { visitor->visit( this ); }
};

// And a circle! Whoa!
class Circle : public Shape {
public:
	float r;
	Circle( float x, float y, float r ) : Shape(x,y), r(r) {}

	// Pretty much the same song and dance.
	void accept( Visitor* visitor ) { visitor->visit( this ); }
};

// For reference, here is a naive implementation of a Visitor.
// While it works, the series of if statements used to figure
// out which class this guy is visiting is a real pain to maintain.
// Don't do it!
class NaiveVisitor {
public:
	virtual void visit(Rectangle* rectangle) = 0;
	virtual void visit(Circle* circle) = 0;

	// doVisit is used by iterating through a collection and,
	// for each Shape, calling the NaiveVisitor's doVisit method.
	// Because no one tells the NaiveVisitor which class it's visiting,
	// it has to do the gross type casting stuff below.
	// Seriously, don't do it!
	void doVisit( Shape* shape ) {

		Rectangle* rec = dynamic_cast<Rectangle*>(shape);
		if ( rec )
			visit( rec );

		Circle* circ = dynamic_cast<Circle*>(shape);
		if ( circ )
			visit( circ );
	}
};

// Here's an implementation of the Visitor.
// It calcultates the area of each shape it visits.
class AreaCalculator : public Visitor {
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

// Here's another implementation.
// It prints out the name of each shape it visits.
class Namer : public Visitor {
public:
	void visit( Rectangle* rectangle ) {
		cout << "name: " << "rectangle" << endl;
	}

	void visit( Circle* circle ) {
		cout << "name: " << "circle" << endl;
	}
};

// And one last implementation.
// This class demonstrates that,
// in addition to "faking" virtual methods,
// a Visitor has state and can be used to
// aggregate information over a collection.
// See below!
class RectangleCounter : public Visitor {
public:
	float count;
	RectangleCounter() : count(0) {}

	void visit(Circle* circle) {}
	void visit(Rectangle* rectangle) { ++count; }
};

// This is just a bit of junk to give us a ShapeList class.
typedef vector<Shape*> ShapeList;

int main() {

	// First, a collection of Shapes is created.
	// Remember, from outside, each of the elements in this collection
	// looks like a Shape. There's no way of looking at them and
	// knowing that they're actually Rectangles and Circles without
	// doing some kind of type inference.
	ShapeList list;
	list.push_back( new Rectangle(0,0,10,20) );
	list.push_back( new Rectangle(10,10, 5,3) );
	list.push_back( new Circle( 5,5, 15) );

	// AreaCalculator demo.
	// An AreaCalculator Visitor is created.
	// Each of the shapes accepts it and then ask it to Visit them.
	AreaCalculator *ac = new AreaCalculator();
	for ( int i = 0; i < list.size(); ++i ) {
		list[i]->accept( ac );
	}
	delete ac;

	// Namer demo.
	Namer *n = new Namer();
	for ( int i = 0; i < list.size(); ++i ) {
		list[i]->accept( n );
	}
	delete n;

	// RectangleCounter demo.
	RectangleCounter *rc = new RectangleCounter();
	for ( int i = 0; i < list.size(); ++i ) {
		list[i]->accept( rc );
	}

	// Note that here, after visiting the Shapes,
	// the state of the RectangleCounter can be queried to
	// determine what it found out while visiting.
	cout << "there are " << rc->count << " rectangles" << endl;
	delete rc;

	// And then, some cleaing up.
	for ( int i = 0; i < list.size(); ++i )
		delete list[i];

	return 0;
}
