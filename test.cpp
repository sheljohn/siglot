#include "siglot.h"
#include <iostream>
#include <string>

using namespace std;
using namespace siglot;

#define PRINT_EVENT_SEPARATOR cout << "----------" << endl;



/**
 * The data structure containing "event information".
 * This will be the template parameter of Signal and Slots.
 * 
 * NOTE: Connections can only occur between Slots and Signals
 * with the same data_type.
 */
struct EventData
{
    string s;
};



/**
 * Declare a Signal using the EventData structure.
 */
Signal<EventData> my_signal;



/**
 * Example of a non-member callback function. 
 *
 * NOTE: The prototype of a non-member callback function is:
 * 
 *     void callback_function( const data_type& data );
 * 
 * UNLESS data_type is VoidData, in which case it is:
 *
 *     void callback_function();
 */
void plain_callback( const EventData& data )
{
    cout << "[Plain]: " << data.s << endl;
}



/**
 * Example of a member callback function.
 */
struct SomeClass
{
    /**
     * Declare a MemberSlot for the class SomeClass using the 
     * EventData structure.
     */
    MemberSlot<SomeClass,EventData> mslot;

    /**
     * The MemberSlot must bind to the current instance and
     * to the member callback function.
     *
     * NOTE: binding can be done upon construction (like here) 
     * or similarly using the method "bind".
     */
    SomeClass(): mslot(this, &SomeClass::member_callback) {}


    void attach() { mslot.listen_to( &my_signal ); }
    void detach() { mslot.detach(); }


    /**
     * Example of a member callback function.
     *
     * NOTE: The same rules as mentionned before apply as far as 
     * the method's signature is concerned.
     */
    void member_callback( const EventData& data )
    {
        cout << "[Member]: " << data.s << endl;
    }
};



    /********************     **********     ********************/
    /********************     **********     ********************/



int main()
{
    /**
     * Declare a non-member callback slot and bind it to plain_callback.
     */
    Slot<EventData> slot(plain_callback);

    /**
     * Instanciate the class with member slot.
     */
    SomeClass my_class;


    /********************     **********     ********************/


    /**
     * Nothing is attached to the signal yet, so not output 
     * should print.
     */
    my_signal.data.s = "None";
    PRINT_EVENT_SEPARATOR
    my_signal.invoke();

    /**
     * Attach the non-member slot to the signal and trigger 
     * the event.
     */
    my_signal.data.s = "Plain only";
    slot.listen_to( &my_signal );
    PRINT_EVENT_SEPARATOR
    my_signal.invoke();

    /**
     * Both Slot and MemberSlot can attach to the same Signal.
     * Here we attach the member slot to the Signal and trigger 
     * the event. Both Slots should be called.
     */
    my_signal.data.s = "Both";
    my_class.attach();
    PRINT_EVENT_SEPARATOR
    my_signal.invoke();

    /**
     * Slot/MemberSlot can be detached using the method "detach".
     * This will deactivate them and remove them from the Signal 
     * callback set.
     */
    my_signal.data.s = "Member only";
    slot.detach();
    PRINT_EVENT_SEPARATOR
    my_signal.invoke();

    /**
     * Alternatively MemberSlots are automatically detached upon 
     * desctruction of the instance holding them. 
     */
    my_signal.data.s = "None";
    my_class.~SomeClass();
    PRINT_EVENT_SEPARATOR
    my_signal.invoke();
}