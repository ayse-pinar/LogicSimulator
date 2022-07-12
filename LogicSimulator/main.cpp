#include <iostream>

#pragma warning(push, 0)
#include <SFML/Graphics.hpp>
#pragma warning(pop)
#define MAX_CONNECTIONS 5

using namespace std;
class Simulator;
class Object;
class LogicElement;
//void removeFromVector(int id, vector<Wire>& vec);

class Object {
    Object* next; 

    bool selected; // if the object is selected
    int ObjID; //unique id to be used in deletion etc.
    sf::Texture textures[2]; //texture of the object
    sf::Sprite sprite; //sprite of the object
    static int ObjNo; //for ObjID to be based on
public:  
    friend class Simulator;
    friend class AndGate;
    friend class LogicElement;
    friend class Wire;
    friend class OrGate;
    friend class XorGate;
    friend class NotGate;
    friend class DFlipFlop;
    friend class Vdd;
    friend class Ground;
    friend class Led;
    friend class Clock;
    //friend void removeFromVector(int id, vector<Wire>& vec);

    Object() //default constructor for object
    { 
        this->ObjID = ObjNo;
        ObjNo += 1;
    } 
    Object(sf::Vector2f& vec, sf::Texture& text) {

        this->ObjID = ObjNo;
        ObjNo += 1;
    }

    //used while running the simulation
    virtual void runner(sf::Event& event, sf::Vector2f& mouse, sf::RenderWindow& window, Simulator& sim) {} 

    //for drawing the objects
    virtual void drawer(sf::RenderWindow& w){}

    //used to see if the object is selected
    virtual bool selection(sf::Vector2f m, sf::RenderWindow& w) { return false; };

    //make adjustments if the object is selected
    virtual void selector(sf::RenderWindow& w) {};

    //perform necessary resets when stop pressed
    virtual void resetter() {};

    //deleting the wires or objects along with their wires
    virtual void WireDelete(sf::RenderWindow& w, Simulator& sim) {};


};
int Object::ObjNo = 0;

enum pinType { INPUT, OUTPUT };
enum pinState { HIGHZ, LOW, HIGH };

class Pin { 
public:
    
private: 

    int index;
    pinType type;
    vector< Pin* > connectedTo;
    vector <Wire*> wires;
    int numConnections;
    sf::Vector2f pos;
    pinState state;
public:      
    static vector<Pin*> allPins; //vector to hold all pins
    friend class Wire;
    friend class LogicElement;
    friend class AndGate;
    friend class OrGate;
    friend class XorGate;
    friend class NotGate;
    friend class DFlipFlop;
    friend class Vdd;
    friend class Ground;
    friend class Led;
    friend class Clock;

    bool selected;
    sf::RectangleShape pinShape;
    Pin() { //default constructor 
    };

    Pin(sf::Vector2f ps, pinType t, int inx, LogicElement& lg);

    //to check if pin is selected
    bool selection(sf::Vector2f& m, sf::RenderWindow& w) {
        sf::FloatRect bounds = this->pinShape.getGlobalBounds();
        if (bounds.contains(m))
        {
            this->selected = true;
            this->protoWire(m, w);
            return true;
        }
        else {
            return false;
        }

    }

    //draws the prototype of the wire
    void protoWire(sf::Vector2f& m, sf::RenderWindow& w) {
        sf::VertexArray lines(sf::LinesStrip, 2);
        lines[0].position = (this->pinShape.getTransform().transformPoint(this->pinShape.getPoint(0).x + 5.f, this->pinShape.getPoint(0).y + 5.f));
        lines[1].position = (m);
        lines[0].color = sf::Color::Black;
        lines[1].color = sf::Color::Black;
        w.draw(lines);
        
    } 

    //set state of the pin
    void setValue(pinState newstate) {
        this->state = newstate;
    } 

    //get state of the pin
    pinState getValue() {
        return this->state;
    } 
};
vector <Pin*> Pin::allPins;

class Wire : public Object {
    sf::Vertex line[2];
    Pin* pins[2];
    
public:
    friend class LogicElement;   
    static vector< Wire* > Wires; //vector to hold all wires
    Wire(Pin& p1, Pin& p2) : Object() {
        //check pinType
        if (p1.type == OUTPUT && p2.type == INPUT) {
            this->pins[0] = &p1;
            this->pins[1] = &p2;

        }
        else if (p1.type == INPUT && p2.type == OUTPUT) {
            this->pins[1] = &p1;
            this->pins[0] = &p2;
        }
        else {
            throw "Incompatible pins.";
        }

        //check if max_connections are exceeded
        if (p1.connectedTo.size() < MAX_CONNECTIONS && p2.connectedTo.size() < MAX_CONNECTIONS) {
            p1.connectedTo.push_back(&p2);
            p2.connectedTo.push_back(&p1);
            p1.numConnections += 1;
            p2.numConnections += 1;
        }
        else {
            throw "Reached maximum connections";
        }

        this->pins[0]->wires.push_back(this);
        this->pins[1]->wires.push_back(this);

    }

    //draws the constructed wires
    void drawWire(sf::Vector2f& m, sf::RenderWindow& w) { 
        Pin P_in = *this->pins[0];
        Pin P_out = *this->pins[1];

        this->line[0].position = (P_in.pinShape.getTransform().transformPoint(P_in.pinShape.getPoint(0).x + 5.f, P_in.pinShape.getPoint(0).y + 5.f));
        this->line[1].position = (P_out.pinShape.getTransform().transformPoint(P_out.pinShape.getPoint(0).x + 5.f, P_out.pinShape.getPoint(0).y + 5.f));
        this->line[0].color = sf::Color::Black;
        this->line[1].color = sf::Color::Black;
        w.draw(line, 2, sf::Lines);
    }

    //redirect to drawWire
    void drawer(sf::Event& event, sf::Vector2f& mouse, sf::RenderWindow& window) { 
        this->drawWire(mouse,window);
    } 

    //equate the output pins' states to connected input pins
    void runner(sf::Event& event, sf::Vector2f& mouse, sf::RenderWindow& window, Simulator& sim) { 
        this->pins[1]->setValue(this->pins[0]->getValue());
    }

    //to select the wire
    bool selection(sf::Vector2f& m, sf::RenderWindow& w) {
        //sf::VertexArray rect(sf::Quads, 4);
        //rect[0].position = this->pins[0]->pinShape.getPosition();
        //rect[1].position = sf::Vector2f(this->pins[0]->pinShape.getPosition().x, this->pins[1]->pinShape.getPosition().y);
        //rect[2].position = sf::Vector2f(this->pins[1]->pinShape.getPosition().x, this->pins[0]->pinShape.getPosition().y);
        //rect[3].position = this->pins[1]->pinShape.getPosition();

        //rect[0].color = sf::Color::Red;
        //rect[1].color = sf::Color::Red;
        //rect[2].color = sf::Color::Red;
        //rect[3].color = sf::Color::Red;

        //w.draw(rect);

        sf::FloatRect bounds = this->sprite.getGlobalBounds();
        if (bounds.contains(m))
        {
            this->selected = true;

            //this->protoWire(m, w);
            return true;
        }
        else {
            this->selected = false;
            return false;
        }
        //return true;

    
    }

    //if wire is selected
    void selector(sf::RenderWindow& w) { 
        this->line[0].color = sf::Color::Red;
        this->line[1].color = sf::Color::Red;
        w.draw(line, 2, sf::Lines);
    }

    //to delete selected wire
    void WireDelete(sf::RenderWindow& w, Simulator& sim); 
};
vector< Wire* > Wire::Wires; //vector to hold all wires

class LogicElement : public Object{   
    
public:
    int numPins;
    Pin* pins[4];
    friend class OrGate;
    friend class XorGate;
    friend class NotGate;
    friend class DFlipFlop;
    friend class Vdd;
    friend class Ground;
    friend class Led;
    friend class Clock;
    friend class AndGate;
    LogicElement(sf::Vector2f vec, sf::Texture& text) : Object (vec, text) {
        this->textures[0] = text;
        this->sprite.setTexture(this->textures[0]);
        this->sprite.setPosition(vec);
        this->sprite.setScale(sf::Vector2f(0.75f, 0.75f));
        this->textures[0].setSmooth(true);
    
    };

    //calculate output of logic element
    virtual void calculate_out() {}; 

    //reset logic element if needed
    virtual void resetter() {}; 

    //perform the simulation - redirect to calculation
    void runner(sf::Event& event, sf::Vector2f& mouse, sf::RenderWindow& window, Simulator& sim) { 
        this->calculate_out();

    };

    //draw the complete logic element
    void drawer(sf::RenderWindow &w) { 
        w.draw(this->sprite);
        for (int i = 0; i < this->numPins; i++) {
            w.draw(this->pins[i]->pinShape);
        }
               
    }

    //returns whether logic element is selected
    bool selection(sf::Vector2f m, sf::RenderWindow& w) { 
        sf::FloatRect bounds = this->sprite.getGlobalBounds();
        if (bounds.contains(m))
        {
            this->selected = true;

            //this->protoWire(m, w);
            return true;
        }
        else {
            this->selected = false;
            return false;
        }
        //return true;
    }

    //if logic element is selected
    virtual void selector(sf::RenderWindow& w) {
        sf::RectangleShape rect;
        rect.setSize(sf::Vector2f(80, 80));
        rect.setFillColor(sf::Color(255, 255, 255, 28));
        rect.setOutlineThickness(3.f);
        rect.setOutlineColor(sf::Color::Red);
        rect.setPosition(this->sprite.getPosition().x-2.f, this->sprite.getPosition().y-15.f);
        w.draw(rect);
    }

    //delete logic element along with wires
    void WireDelete(sf::RenderWindow& w, Simulator& sim); 

};

class Simulator {
    Object* object;
    Object* head;
public:
    friend class Object;
    Simulator() {
        this->head = NULL;
    }

    //add new object to linked list
    bool addObject(Object& o) { 
        Object* prev, * current , * new_object;
        Object* ptr = head;
        new_object = &o;
        if (!new_object) return false;
        while (ptr) {
                if (ptr->ObjID == o.ObjID) { //do not add the same element twice
                    return false;
                    break;
                }
                ptr = ptr->next;
        }
        if (head)
        {
            prev = head;
            current = head->next;           

            while (current)
            {
                prev = current;
                current = current->next;
            }
            prev->next = new_object;
        }
        else
            head = new_object;
        return true;
    }; 

    //print object ID's
    void printObjects() { 
        Object* ptr = head;
        if (head == NULL) {
            cout << "List is empty!" << endl;
            return;
        }
        while (ptr) {
            cout << ptr->ObjID << endl;
            //w.draw(ptr->sprite);
            ptr = ptr->next;
        }
        cout << "***" << endl;
    }

    //draw all objects
    void drawObjects(sf::RenderWindow& w) { 
        Object* ptr = head;
        if (head == NULL) {
            return;
        }
        while (ptr) {
            ptr->drawer(w);
            ptr = ptr->next;
        }
    }

    //check if an object is being selected
    Object* ObjSelector(sf::Event& event, sf::Vector2f& mouse, sf::RenderWindow& window) { 
        Object* ptr = head;
        while (ptr) {
            if (ptr->selection(mouse, window)) {
                return ptr;
            }
            ptr = ptr->next;
        }
        return NULL;
    }

    //delete object - remove from list
    void delObj(Object& o) { 
        Object* ptr = head;
        Object* prev = NULL;
        if (head == NULL) {
            return;
        }
        while (ptr) {
            if (ptr->ObjID == o.ObjID) {
                if (prev)
                    prev->next = ptr->next;
                else
                    head = ptr->next;
                delete ptr;
                return;
            }
            prev = ptr;
            ptr = ptr->next;
        }
    }

    //when stop is pressed
    void reset() {
        Object* ptr = head;
        if (head == NULL) {
            return;
        }
        while (ptr) {
            ptr->resetter();
            //window.draw(ptr->sprite);
            ptr = ptr->next;
        }
    }
 
    //when start is pressed
    void run(sf::Event& event, sf::Vector2f& mouse, sf::RenderWindow& window) { //run simulation
        Object* ptr = head;
        if (head == NULL) {
            return;
        }
        while (ptr) {
            ptr->runner(event, mouse, window, *this);
            ptr = ptr->next;
        }
    }
};

//Gate Classes
#pragma region
class AndGate : public LogicElement {

public:
    void calculate_out();
    friend class Pin;
    AndGate(sf::Vector2f vec, sf::Texture& text, sf::RenderWindow& w) : LogicElement(vec, text) {
        this->numPins = 3;
    }
    void selector(sf::RenderWindow& w) {
        sf::RectangleShape rect;
        rect.setSize(sf::Vector2f(80, 80));
        rect.setFillColor(sf::Color(255, 255, 255, 28));
        rect.setOutlineThickness(3.f);
        rect.setOutlineColor(sf::Color(250, 0, 0));
        rect.setPosition(this->sprite.getPosition().x - 2.f, this->sprite.getPosition().y - 15.f);
        w.draw(rect);
    }
    
};

void AndGate::calculate_out()
{
    if (this->pins[0]->state == HIGH && this->pins[1]->state == HIGH)
    {
        this->pins[2]->state = HIGH;
    }
    else
    {
        this->pins[2]->state = LOW;
    }
}

class OrGate : public LogicElement {

public:
    void calculate_out();
    friend class Pin;
    OrGate(sf::Vector2f vec, sf::Texture& text, sf::RenderWindow& w) : LogicElement(vec, text) {
        this->numPins = 3;
    }

    void selector(sf::RenderWindow& w) {
        sf::RectangleShape rect;
        rect.setSize(sf::Vector2f(80, 80));
        rect.setFillColor(sf::Color(255, 255, 255, 28));
        rect.setOutlineThickness(3.f);
        rect.setOutlineColor(sf::Color(250, 0, 0));
        rect.setPosition(this->sprite.getPosition().x - 2.f, this->sprite.getPosition().y - 15.f);
        w.draw(rect);
    }
};

void OrGate::calculate_out()
{
    if (this->pins[0]->state == LOW && this->pins[1]->state == LOW)
    {
        this->pins[2]->state = LOW;
    }
    else
    {
        this->pins[2]->state = HIGH;
    }
}

class XorGate : public LogicElement {

public:
    void calculate_out();
    friend class Pin;
    XorGate(sf::Vector2f vec, sf::Texture& text, sf::RenderWindow& w) : LogicElement(vec, text) {
        this->numPins = 3;
    }

    void selector(sf::RenderWindow& w) {
        sf::RectangleShape rect;
        rect.setSize(sf::Vector2f(80, 80));
        rect.setFillColor(sf::Color(255, 255, 255, 28));
        rect.setOutlineThickness(3.f);
        rect.setOutlineColor(sf::Color(250, 0, 0));
        rect.setPosition(this->sprite.getPosition().x - 2.f, this->sprite.getPosition().y - 15.f);
        w.draw(rect);
    }
};

void XorGate::calculate_out()
{
    if (this->pins[0]->state == HIGH && this->pins[1]->state == HIGH)
    {
        this->pins[2]->state = LOW;
    }
    else if (this->pins[0]->state == LOW && this->pins[1]->state == LOW)
    {
        this->pins[2]->state = LOW;
    }
    else
    {
        this->pins[2]->state = HIGH;
    }
}

class NotGate : public LogicElement {

public:
    void calculate_out();
    friend class Pin;
    NotGate(sf::Vector2f vec, sf::Texture& text, sf::RenderWindow& w) : LogicElement(vec, text) {
        this->numPins = 2;
    }

    void selector(sf::RenderWindow& w) {
        sf::RectangleShape rect;
        rect.setSize(sf::Vector2f(80, 80));
        rect.setFillColor(sf::Color(255, 255, 255, 28));
        rect.setOutlineThickness(3.f);
        rect.setOutlineColor(sf::Color(250, 0, 0));
        rect.setPosition(this->sprite.getPosition().x - 2.f, this->sprite.getPosition().y - 15.f);
        w.draw(rect);
    }
};

void NotGate::calculate_out()
{
    if (this->pins[0]->state == HIGH)
    { 
        this->pins[1]->state = LOW;
    }
    else 
    { 
        this->pins[1]->state = HIGH;
    }
}

class DFlipFlop : public LogicElement {

public:
    friend class Pin;
    DFlipFlop(sf::Vector2f vec, sf::Texture& text, sf::RenderWindow& w) : LogicElement(vec, text) {
        this->numPins = 4;
    }
    void calculate_out();

    void selector(sf::RenderWindow& w) {
        sf::RectangleShape rect;
        rect.setSize(sf::Vector2f(80, 80));
        rect.setFillColor(sf::Color(255, 255, 255, 28));
        rect.setOutlineThickness(3.f);
        rect.setOutlineColor(sf::Color(250, 0, 0));
        rect.setPosition(this->sprite.getPosition().x - 2.f, this->sprite.getPosition().y - 15.f);
        w.draw(rect);
    }
};

void DFlipFlop::calculate_out()
{
    if (this->pins[0]->state == HIGH && this->pins[1]->state == HIGH)
    {
        this->pins[2]->state = HIGH;
        this->pins[3]->state = LOW;
    }
    else
    {
        this->pins[2]->state = LOW;
        this->pins[3]->state = HIGH;
    }
}

class Clock : public LogicElement {

public:
    friend class Pin;
    void calculate_out();
    sf::Clock clk;
    sf::Time elapsed;
    sf::Time t1 = sf::milliseconds(1000);
    Clock(sf::Vector2f vec, sf::Texture& text, sf::RenderWindow& w) : LogicElement(vec, text) {
        this->numPins = 1;

        clk.restart();
    }

    void selector(sf::RenderWindow& w) {
        sf::RectangleShape rect;
        rect.setSize(sf::Vector2f(65, 65));
        rect.setFillColor(sf::Color(255, 255, 255, 28));
        rect.setOutlineThickness(3.f);
        rect.setOutlineColor(sf::Color(250, 0, 0));
        rect.setPosition(this->sprite.getPosition().x - 2.f, this->sprite.getPosition().y - 10.f);
        w.draw(rect);
    }
};

void Clock::calculate_out()
{
    elapsed = clk.getElapsedTime();
    if (elapsed >= t1) {

        if (this->pins[0]->state == LOW)
        {
            this->pins[0]->state = HIGH;
        }
        else
        {
            this->pins[0]->state = LOW;
        }
        clk.restart();
    }
}

class Led : public LogicElement {

public:
    friend class Pin;
    
    Led(sf::Vector2f vec, sf::Texture& text, sf::Texture& text2, sf::RenderWindow& w) : LogicElement(vec, text) {
        this->numPins = 2;
        this->textures[1] = text2;
        this->textures[1].setSmooth(true);
    }
    void calculate_out();
    void selector(sf::RenderWindow& w) {
        sf::RectangleShape rect;
        rect.setSize(sf::Vector2f(40, 75));
        rect.setFillColor(sf::Color(255, 255, 255, 28));
        rect.setOutlineThickness(3.f);
        rect.setOutlineColor(sf::Color(250, 0, 0));
        rect.setPosition(this->sprite.getPosition().x-5.f, this->sprite.getPosition().y - 10.f);
        w.draw(rect);
    }
    void resetter() {
        this->sprite.setTexture(textures[0]);
    }
};

void Led::calculate_out()
{
    if (this->pins[0]->state == HIGH)
    {
        this->sprite.setTexture(textures[1]);
    }
    else if (this->pins[0]->state == LOW)
    {
        this->sprite.setTexture(textures[0]);
    }
    else
    {
        cout << this->pins[0]->getValue() << endl;
        cout << "LED Calculation failed internally" << endl;
    }
}

class Vdd : public LogicElement {

public:
    friend class Pin;
    void calculate_out();
    Vdd (sf::Vector2f vec, sf::Texture& text, sf::RenderWindow& w) : LogicElement(vec, text) {
        this->numPins = 1;
    }

    void selector(sf::RenderWindow& w) {
        sf::RectangleShape rect;
        rect.setSize(sf::Vector2f(65, 65));
        rect.setFillColor(sf::Color(255, 255, 255, 28));
        rect.setOutlineThickness(3.f);
        rect.setOutlineColor(sf::Color(250, 0, 0));
        rect.setPosition(this->sprite.getPosition().x - 10.f, this->sprite.getPosition().y - 10.f);
        w.draw(rect);
    }
};

void Vdd::calculate_out()
{
    this->pins[0]->setValue(HIGH);
}

class Ground : public LogicElement {

public:
    friend class Pin;
    void calculate_out();
    Ground(sf::Vector2f vec, sf::Texture& text, sf::RenderWindow& w) : LogicElement(vec, text) {
        this->numPins = 1;
    }

    void selector(sf::RenderWindow& w) {
        sf::RectangleShape rect;
        rect.setSize(sf::Vector2f(55, 55));
        rect.setFillColor(sf::Color(255, 255, 255, 28));
        rect.setOutlineThickness(3.f);
        rect.setOutlineColor(sf::Color(250, 0, 0));
        rect.setPosition(this->sprite.getPosition().x - 10.f, this->sprite.getPosition().y - 15.f);
        w.draw(rect);
    }
};

void Ground::calculate_out()
{
    this->pins[0]->state = LOW;
}
#pragma endregion

Pin::Pin(sf::Vector2f ps, pinType t, int inx, LogicElement& lg) {
    this->pos = ps;
    this->pinShape = sf::RectangleShape(sf::Vector2f(10.f, 10.f));
    this->pinShape.setPosition(ps);
    this->pinShape.setFillColor(sf::Color::Red);
    this->type = t;
    this->index = inx;
    this->selected = false;
    allPins.push_back(this);
    lg.pins[index] = this;
    this->state = LOW;
}

void Wire::WireDelete(sf::RenderWindow& w, Simulator& sim) {
    //removeFromVector()
    sim.delObj(*this);
}
void LogicElement::WireDelete(sf::RenderWindow& w, Simulator& sim) {
    Wire* wptr = NULL;
    for (int i = 0; i < this->numPins; i++) {
        for (int j = 0; j < this->pins[i]->wires.size(); j++) {
            wptr = this->pins[i]->wires[j];
            for (int i = 0; i < Wire::Wires.size(); i++) {
                if (Wire::Wires[i]->ObjID == wptr->ObjID) {
                    Wire::Wires.erase(Wire::Wires.begin() + i);
                }
            }
            sim.delObj(*wptr);
        }
    }   
    sim.delObj(*this);
}


int main()
{
    //background setting
#pragma region
    sf::RenderWindow window(sf::VideoMode(1600, 900), "Logic Simulator");
    sf::RectangleShape background(sf::Vector2f(1600.f, 900.f));
    background.setFillColor(sf::Color(255, 255, 250));

    sf::RectangleShape bar(sf::Vector2f(125.f, 900.f));
    bar.setFillColor(sf::Color(211, 211, 211));

    sf::RectangleShape bar2(sf::Vector2f(1600.f, 75.f));
    bar2.setFillColor(sf::Color(211, 211, 211));
    sf::FloatRect boundsbar = bar.getGlobalBounds();
    sf::FloatRect boundsbar2 = bar2.getGlobalBounds();

    sf::Texture andgate;
    if (!andgate.loadFromFile("../assets/AND.png"))
    {
        // error...
    }
    sf::Sprite AND;
    AND.setTexture(andgate);
    AND.setOrigin(-30.f, -50.f);
    AND.setScale(sf::Vector2f(0.75f, 0.75f));
    andgate.setSmooth(true);
    sf::FloatRect boundsAND = AND.getGlobalBounds();

    sf::Texture orgate;
    if (!orgate.loadFromFile("../assets/OR.png"))
    {
        // error...
    }
    sf::Sprite OR;
    OR.setTexture(orgate);
    OR.setOrigin(-30.f, -150.f);
    OR.setScale(sf::Vector2f(0.75f, 0.75f));
    orgate.setSmooth(true);
    sf::FloatRect boundsOR = OR.getGlobalBounds();

    sf::Texture xorgate;
    if (!xorgate.loadFromFile("../assets/XOR.png"))
    {
        // error...
    }
    sf::Sprite XOR;
    XOR.setTexture(xorgate);
    XOR.setOrigin(-30.f, -250.f);
    XOR.setScale(sf::Vector2f(0.75f, 0.75f));
    xorgate.setSmooth(true);
    sf::FloatRect boundsXOR = XOR.getGlobalBounds();

    sf::Texture notgate;
    if (!notgate.loadFromFile("../assets/NOT.png"))
    {
        // error...
    }
    sf::Sprite NOT;
    NOT.setTexture(notgate);
    NOT.setOrigin(-30.f, -350.f);
    NOT.setScale(sf::Vector2f(0.75f, 0.75f));
    notgate.setSmooth(true);
    sf::FloatRect boundsNOT = NOT.getGlobalBounds();

    sf::Texture dflipflop;
    if (!dflipflop.loadFromFile("../assets/DFF.png"))
    {
        // error...
    }
    sf::Sprite DFF;
    DFF.setTexture(dflipflop);
    DFF.setOrigin(-30.f, -450.f);
    DFF.setScale(sf::Vector2f(0.75f, 0.75f));
    dflipflop.setSmooth(true);
    sf::FloatRect boundsDFF = DFF.getGlobalBounds();

    sf::Texture clock;
    if (!clock.loadFromFile("../assets/CLOCK.png"))
    {
        // error...
    }
    sf::Sprite CLK;
    CLK.setTexture(clock);
    CLK.setOrigin(-45.f, -560.f);
    CLK.setScale(sf::Vector2f(0.75f, 0.75f));
    clock.setSmooth(true);
    sf::FloatRect boundsCLK = CLK.getGlobalBounds();

    sf::Texture vdd;
    if (!vdd.loadFromFile("../assets/VDD.png"))
    {
        // error...
    }
    sf::Sprite VDD;
    VDD.setTexture(vdd);
    VDD.setOrigin(-45.f, -660.f);
    VDD.setScale(sf::Vector2f(0.75f, 0.75f));
    vdd.setSmooth(true);
    sf::FloatRect boundsVDD = VDD.getGlobalBounds();

    sf::Texture led;
    if (!led.loadFromFile("../assets/LEDOFF.png"))
    {
        // error...
    }
    sf::Sprite LED;
    LED.setTexture(led);
    LED.setOrigin(-55.f, -760.f);
    LED.setScale(sf::Vector2f(0.75f, 0.75f));
    led.setSmooth(true);
    sf::FloatRect boundsLED = LED.getGlobalBounds();

    sf::Texture ledon;
    if (!ledon.loadFromFile("../assets/LEDON.png"))
    {
        // error...

    }
    sf::Sprite LEDON;
    LEDON.setTexture(ledon);
    LEDON.setOrigin(-55.f, -760.f);
    LEDON.setScale(sf::Vector2f(0.75f, 0.75f));
    ledon.setSmooth(true);
    sf::FloatRect boundsLEDON = LEDON.getGlobalBounds();

    sf::Texture ground;
    if (!ground.loadFromFile("../assets/GND.png"))
    {
        // error...
    }
    sf::Sprite GND;
    GND.setTexture(ground);
    GND.setOrigin(-55.f, -880.f);
    GND.setScale(sf::Vector2f(0.75f, 0.75f));
    ground.setSmooth(true);
    sf::FloatRect boundsGND = GND.getGlobalBounds();

    sf::Texture start;
    if (!start.loadFromFile("../assets/START.png"))
    {
        // error...
    }
    sf::Sprite START;
    START.setTexture(start);
    START.setOrigin(-200.f, -30.f);
    START.setScale(sf::Vector2f(0.75f, 0.75f));
    start.setSmooth(true);
    sf::FloatRect boundsSTART = START.getGlobalBounds();

    sf::Texture stop;
    if (!stop.loadFromFile("../assets/STOP.png"))
    {
        // error...
    }
    sf::Sprite STOP;
    STOP.setTexture(stop);
    STOP.setOrigin(-500.f, -30.f);
    STOP.setScale(sf::Vector2f(0.75f, 0.75f));
    stop.setSmooth(true);
    sf::FloatRect boundsSTOP = STOP.getGlobalBounds();

    sf::Sprite AND2 = AND;
    sf::Sprite OR2 = OR;
    sf::Sprite XOR2 = XOR;
    sf::Sprite NOT2 = NOT;
    sf::Sprite DFF2 = DFF;
    sf::Sprite CLK2 = CLK;
    sf::Sprite VDD2 = VDD;
    sf::Sprite LED2 = LED;
    sf::Sprite GND2 = GND;
#pragma endregion   

    //initialization of values
#pragma region
    bool mouseHold = false; //if mouse is holding something - for actions strated by clicking sth
    bool Run = false; //if simulation is running - flag toggled by clicking start/stop
    
    Pin* selpin = NULL; //selected pin
    Object* selel = NULL; //selected object
    sf::Sprite* selbut = NULL; //selected button

    Simulator List; //main object list

    //Individual object pointers
    AndGate* A = NULL;
    OrGate* O = NULL;
    Clock* C = NULL;
    DFlipFlop* D = NULL;
    Led* L = NULL;
    NotGate* N = NULL;
    Ground* G = NULL;
    Vdd* V = NULL;
    XorGate* X = NULL;    
    Wire* wireptr = NULL;
        
    sf::Vector2f inposi; //initial position of sprite
#pragma endregion

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();


        //background draw
#pragma region
        window.draw(background);
        window.draw(bar);
        window.draw(bar2);
        window.draw(AND);
        window.draw(OR);
        window.draw(XOR);
        window.draw(NOT);
        window.draw(DFF);
        window.draw(CLK);
        window.draw(VDD);
        window.draw(LED);
        window.draw(GND);
        window.draw(START);
        window.draw(STOP);
#pragma endregion

        sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                //check if a pin is selected for starting a wire
                for (int i = 0; i < Pin::allPins.size(); i++) {
                    if (Pin::allPins[i]->selection(mouse, window)) {
                        mouseHold = true;
                        selpin = Pin::allPins[i];
                        break;
                    }
                }

                //check if a button is selected for new object creation
#pragma region
                if (boundsAND.contains(mouse)) {
                    mouseHold = true;
                    AND2.setColor(sf::Color(255, 255, 255, 255));
                    selbut = &AND2;
                    inposi = sf::Vector2f(20.f, 40.f);
                }
                if (boundsOR.contains(mouse)) {
                    mouseHold = true;
                    OR2.setColor(sf::Color(255, 255, 255, 255));
                    selbut = &OR2;
                    inposi = sf::Vector2f(20.f, 110.f);
                }
                if (boundsCLK.contains(mouse)) {
                    mouseHold = true;
                    CLK2.setColor(sf::Color(255, 255, 255, 255));
                    selbut = &CLK2;
                    inposi = sf::Vector2f(30.f, 420.f);
                    
                }
                if (boundsDFF.contains(mouse)) {
                    mouseHold = true;
                    DFF2.setColor(sf::Color(255, 255, 255, 255));
                    selbut = &DFF2;
                    inposi = sf::Vector2f(20.f, 340.f);
                }
                if (boundsGND.contains(mouse)) {
                    mouseHold = true;
                    GND2.setColor(sf::Color(255, 255, 255, 255));
                    selbut = &GND2;
                    inposi = sf::Vector2f(40.f, 660.f);
                }
                if (boundsLED.contains(mouse)) {
                    mouseHold = true;
                    LED2.setColor(sf::Color(255, 255, 255, 255));
                    selbut = &LED2;
                    inposi = sf::Vector2f(35.f, 570.f);
                }
                if (boundsNOT.contains(mouse)) {
                    mouseHold = true;
                    NOT2.setColor(sf::Color(255, 255, 255, 255));
                    selbut = &NOT2;
                    inposi = sf::Vector2f(20.f, 265.f);
                }
                if (boundsVDD.contains(mouse)) {
                    mouseHold = true;
                    VDD2.setColor(sf::Color(255, 255, 255, 255));
                    selbut = &VDD2;
                    inposi = sf::Vector2f(30.f, 490.f);
                }
                if (boundsXOR.contains(mouse)) {
                    mouseHold = true;
                    XOR2.setColor(sf::Color(255, 255, 255, 255));
                    selbut = &XOR2;
                    //selbut->setOrigin(XOR.getOrigin());
                    inposi = sf::Vector2f(20.f, 185.f);

                }
                if (selbut) {
                }
#pragma endregion

                //check if an existing object is selected for deletion
                selel = List.ObjSelector(event, mouse, window);
                if (selel) {
                    mouseHold = true;
                }

                //check if start or stop are pressed
                if (boundsSTART.contains(mouse)) {
                    Run = true;
                }
                if (boundsSTOP.contains(mouse)) {
                    Run = false;
                    List.reset();
                }
            }
        }

        if (mouseHold) {
            if (selel) {
                selel->selector(window);
                if (event.type == sf::Event::KeyPressed)
                {
                    if (event.key.code == sf::Keyboard::Delete)
                    {
                        selel->WireDelete(window, List);
                        selel = NULL;
                    }
                }

            }
            if (selpin) {
                selpin->protoWire(mouse, window);
                if (event.type == sf::Event::MouseButtonReleased)
                {
                    if (event.mouseButton.button == sf::Mouse::Left)
                    {
                        for (int i = 0; i < Pin::allPins.size(); i++) {
                            if (Pin::allPins[i] != selpin && Pin::allPins[i]->selection(mouse, window)) {
                                try {
                                    wireptr = new Wire(*selpin, *Pin::allPins[i]);
                                    Wire::Wires.push_back(wireptr);
                                    List.addObject(*wireptr);
                                }
                                catch (const char* err) {
                                    cout << err << endl;
                                }
                                break;
                            }
                        }
                    }
                    mouseHold = false;
                    selpin = NULL;
                }
            }


            if (selbut) {
                selbut->setPosition(mouse-inposi);
                if (event.type == sf::Event::MouseButtonReleased)
                {
                    if (event.mouseButton.button == sf::Mouse::Left)
                    {
                        if (boundsbar.contains(mouse) == false && boundsbar2.contains(mouse) == false) {
                            //forming new element of selected kind
#pragma region
                            if (selbut == &AND2) {
                                A = new AndGate(mouse, andgate, window);

                                sf::Vector2f andtopinposi(mouse.x, mouse.y + 7.f);
                                sf::Vector2f andbotinposi(mouse.x, mouse.y + 30.f);
                                sf::Vector2f andoutposi(mouse.x + 66.f, mouse.y + 17.f);

                                A->pins[0] = new Pin(andtopinposi, INPUT, 0, *A);
                                A->pins[1] = new Pin(andbotinposi, INPUT, 1, *A);
                                A->pins[2] = new Pin(andoutposi, OUTPUT, 2, *A);
                                List.addObject(*A);
                            }
                            if (selbut == &OR2) {
                                O = new OrGate(mouse, orgate, window);


                                sf::Vector2f ortopinposi(mouse.x, mouse.y + 7.f);
                                sf::Vector2f orbotinposi(mouse.x, mouse.y + 30.f);
                                sf::Vector2f oroutposi(mouse.x + 66.f, mouse.y + 17.f);

                                O->pins[0] = new Pin(ortopinposi, INPUT, 0, *O);
                                O->pins[1] = new Pin(orbotinposi, INPUT, 1, *O);
                                O->pins[2] = new Pin(oroutposi, OUTPUT, 2, *O);
                                List.addObject(*O);
                            }
                            if (selbut == &DFF2) {
                                D = new DFlipFlop(mouse, dflipflop, window);


                                sf::Vector2f dfftopinposi(mouse.x, mouse.y + 10.f);
                                sf::Vector2f dffbotinposi(mouse.x, mouse.y + 25.f);
                                sf::Vector2f dfftopoutposi(mouse.x + 66.f, mouse.y + 10.f);
                                sf::Vector2f dffbotoutposi(mouse.x + 66.f, mouse.y + 40.f);

                                D->pins[0] = new Pin(dfftopinposi, INPUT, 0, *D);
                                D->pins[1] = new Pin(dffbotinposi, INPUT, 1, *D);
                                D->pins[2] = new Pin(dfftopoutposi, OUTPUT, 2, *D);
                                D->pins[3] = new Pin(dffbotoutposi, OUTPUT, 3, *D);

                                List.addObject(*D);
                            }
                            if (selbut == &CLK2) {
                                C = new Clock(mouse, clock, window);

                                sf::Vector2f clkoutposi(mouse.x + 50.f, mouse.y + 17.f);

                                C->pins[0] = new Pin(clkoutposi, OUTPUT, 0, *C);
                                List.addObject(*C);
                            }
                            if (selbut == &GND2) {
                                G = new Ground(mouse, ground, window);


                                sf::Vector2f gndpinposi(mouse.x + 12.f, mouse.y - 1.f);
                                G->pins[0] = new Pin(gndpinposi, OUTPUT, 0, *G);
                                G->pins[0]->setValue(LOW);
                                List.addObject(*G);
                            }
                            if (selbut == &LED2) {
                                L = new Led(mouse, led, ledon, window);


                                sf::Vector2f ledlinposi(mouse.x + 17.f, mouse.y + 50.f);
                                sf::Vector2f ledhinposi(mouse.x + 5.f, mouse.y + 50.f);

                                L->pins[0] = new Pin(ledhinposi, INPUT, 0, *L);
                                L->pins[1] = new Pin(ledlinposi, INPUT, 1, *L);
                                List.addObject(*L);
                            }
                            if (selbut == &NOT2) {
                                N = new NotGate(mouse, notgate, window);


                                sf::Vector2f notinposi(mouse.x, mouse.y + 17.f);
                                sf::Vector2f notoutposi(mouse.x + 66.f, mouse.y + 17.f);

                                N->pins[0] = new Pin(notinposi, INPUT, 0, *N);
                                N->pins[1] = new Pin(notoutposi, OUTPUT, 1, *N);
                                List.addObject(*N);
                            }
                            if (selbut == &VDD2) {
                                V = new Vdd(mouse, vdd, window);

                                sf::Vector2f vddpinposi(mouse.x + 18.f, mouse.y + 36.f);
                                V->pins[0] = new Pin(vddpinposi, OUTPUT, 0, *V);
                                V->pins[0]->setValue(HIGH);
                                List.addObject(*V);
                            }
                            if (selbut == &XOR2) {
                                X = new XorGate(mouse, xorgate, window);

                                sf::Vector2f xortopinposi(mouse.x, mouse.y + 7.f);
                                sf::Vector2f xorbotinposi(mouse.x, mouse.y + 30.f);
                                sf::Vector2f xoroutposi(mouse.x + 66.f, mouse.y + 17.f);

                                X->pins[0] = new Pin(xortopinposi, INPUT, 0, *X);
                                X->pins[1] = new Pin(xorbotinposi, INPUT, 1, *X);
                                X->pins[2] = new Pin(xoroutposi, OUTPUT, 2, *X);
                                List.addObject(*X);

                            }
#pragma endregion
                        }
                        selbut->setColor(sf::Color(255, 255, 255, 0));
                        selbut->setPosition(inposi);
                        mouseHold = false;
                        selbut = NULL;
                    }
                }
            }

        }

        if (selbut) {
            window.draw(*selbut);
        }

        Wire::Wires.shrink_to_fit();
        for (int i = 0; i < Wire::Wires.size(); i++) {

            Wire::Wires[i]->drawWire(mouse, window);
        }

        List.drawObjects(window);

        if (Run) {
            List.run(event, mouse, window);        
            cout << "Running" << endl;
        }

        List.drawObjects(window);

        window.display();
    }
    return 0;
}