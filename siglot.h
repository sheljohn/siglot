#ifndef __SIGLOT__
#define __SIGLOT__

#include <set>
#include <type_traits>

//=============================================
// @filename     siglot.h
// @author       Sheljohn (Jonathan H)
// @contact      Jonathan.hadida@dtc.ox.ac.uk
// @license      Creative Commons by-sa 3.0 
//               http://creativecommons.org/licenses/by-sa/3.0/
//=============================================

namespace siglot
{

/**
 * Empty data structure for Signals without data.
 */
struct VoidData {};



/**
 * The Slot interface as seen by a Signal object.
 */
template <typename data_type>
class CallbackInterface
{
public:

	CallbackInterface(): active(false) {}

protected:

	template <typename U> friend class Signal;

	// Is the Callback subscribed to a Signal?
	mutable bool active;

	// Used upon Signal destruction to deactivate subscribed Slots
	inline void _deactivate() { active = false; }

	// Trigger the callback function
	virtual void operator() ( const data_type& data ) =0;
};



/**
 * The Signal interface as seen by a Slot object.
 * 
 * A Signal stores its subscribers (Slots) as a set of pointers to 
 * CallbackInterfaces. This allows to avoid duplicates, and to access 
 * specific Slots (eg for deactivation) in logarithmic time.
 *
 * This interface provides the methods:
 * - _subscribe  : a new Slot subscribes to the Signal
 * - _unsubscribe: a specific Slot unsubscribes
 * - count       : returns the number of currently subscribed Slots
 */
template <typename data_type>
class SlotSet
{
public:

	typedef CallbackInterface<data_type> slot_type;
	typedef slot_type* slot_ptr;
	typedef SlotSet<data_type> self;

	// Count number of slots currently subscribed
	inline unsigned count() const { return slots.size(); }

protected:

	template <typename U> friend class ListenerInterface;

	// Copy the list of slots
	inline void _copy( const self *other )
	{
		if ( other != this ) slots = other->slots;
	}

	// Insertion/deletion in the slot set
	std::set<slot_ptr> slots;
	inline void _subscribe( slot_ptr s ) { slots.insert(s); }
	inline void _unsubsribe( slot_ptr s ) { slots.erase(s); }
};



/**
 * Defines Slot-actions related to a Signal:
 * - unsubscribe: from registered Signal
 * - subscribe  : to Signal
 */
template <typename data_type>
class ListenerInterface 
	: public CallbackInterface<data_type>
{
public:

	typedef SlotSet<data_type> signal_type;
	typedef signal_type* signal_ptr;
	typedef ListenerInterface<data_type> self;

	// Ask Signal to remove the current Slot from its set
	// and switch to "inactive" state
	void unsubscribe()
	{
		if ( _is_active() ) signal->_unsubsribe(this);
		this->_deactivate();
		signal = nullptr;
	}

	// Subscribe to a specific signal
	void subscribe( signal_ptr s )
	{
		signal = s;
		s->_subscribe(this);
		this->active = true;
	}

	// Check current state
	inline virtual bool is_active() const { return _is_active(); }

protected:

	// Internal inherited methods to check the current state
	inline bool _is_active() const { return this->active = (this->active && signal); }

	// Copy the Signal registered in a sibling
	void _copy( const self *other )
	{
		if ( other != this && other->_is_active() )
			subscribe( other->signal );
	}

	signal_ptr signal;
};



/**
 * Callback objects.
 * These define the types of callback functions for both Slots and MemberSlots,
 * whether the data type is void or not, and provide proxy to the corresponding 
 * function call.
 */
template <typename data_type = VoidData> struct SlotCallback
{
	typedef void (*type)( const data_type& );

	inline static void callback( const data_type& data, type cb ) { (*cb)(data); }
};
template <> struct SlotCallback<VoidData>
{
	typedef void (*type)();

	inline static void callback( const VoidData& data, type cb ) { (*cb)(); }
};

template <typename handle_type, typename data_type = VoidData> struct MemberSlotCallback
{
	typedef void (handle_type::*type)( const data_type& );

	inline static void callback( const data_type& data, handle_type *H, type cb ) { (H->*cb)(data); }
};
template <typename handle_type> struct MemberSlotCallback<handle_type,VoidData>
{
	typedef void (handle_type::*type)();

	inline static void callback( const VoidData& data, handle_type *H, type cb ) { (H->*cb)(); }
};



	/********************     **********     ********************/
	/********************     **********     ********************/



/**
 * The Signal class to use in your code.
 * The template argument corresponds to the type of the "event data structure".
 * The method "invoke" triggers the callback functions of all subscribed Slots.
 */
template <typename data_type = VoidData>
class Signal 
	: public SlotSet<data_type>
{
public:

	typedef Signal<data_type> self;

	data_type data;

	Signal() {}
	~Signal() { clear(); }

	// Nothing is done by default on copy/assignment
	Signal( const self& other ) {}
	self& operator= ( const self& other ) {}

	// Copy of the slots set should be explicitly called
	inline void copy( const self& other ) { this->_copy( &other ); }

	// Disconnect all slots on cleanup
	void clear()
	{ 
		for ( auto slot : this->slots ) slot->_deactivate();
		this->slots.clear();
	}

	// Trigger the signal and invoke all callback functions
	void invoke() const
	{
		for ( auto slot : this->slots ) (*slot)(data);
	}
};



/**
 * Slots are used with NON-MEMBER functions callbacks:
 * - bind     : binds the Slot to a callback function
 * - is_active: checks the current Slot state
 *
 * NOTE:
 * The callback function must have the following signature
 * void callback_function( const data_type& data )
 */
template <typename data_type = VoidData>
class Slot 
	: public ListenerInterface<data_type>
{
public:

	typedef Slot<data_type> self;
	typedef typename SlotCallback<data_type>::type callback_type;

	Slot() { clear(); }
	Slot( callback_type f ) { clear(); bind(f); }
	~Slot() { clear(); }

	Slot( const self& other ) { clear(); copy(other); }
	self& operator= ( const self& other ) { copy(other); }

	void clear() 
	{ 
		this->unsubscribe(); 
		callback = nullptr;
	}

	void bind( callback_type f ) { callback = f; }
	inline bool is_active() const { return callback && this->_is_active(); }

protected:

	void copy( const self& other )
	{
		if ( &other != this && other.is_active() )
		{
			callback = other.callback;
			this->_copy( &other ); 
		}
	}

	inline void operator() ( const data_type& data ) 
	{ SlotCallback<data_type>::callback(data,callback); }
	
	callback_type callback;
};



/**
 * MemberSlots are used with MEMBER-function callbacks:
 * - bind     : bind to an instance and its member callback function
 * - is_active: checks the current Slot state
 *
 * NOTE: The binding function is used as follows:
 * MemberSlot<handle_type,data_type> slot;
 * slot.bind(this, &handle_type::callback_function);
 */
template <typename handle_type, typename data_type = VoidData>
class MemberSlot 
	: public ListenerInterface<data_type>
{
public:

	typedef MemberSlot<handle_type,data_type> self;
	typedef handle_type* handle_ptr;
	typedef const data_type& data_input;
	typedef typename MemberSlotCallback<handle_type,data_type>::type callback_type;

	MemberSlot() { clear(); }
	MemberSlot( handle_ptr h, callback_type f ) { clear(); bind(h,f); }
	~MemberSlot() { clear(); }

	MemberSlot( handle_ptr h, const self& other ) { clear(); copy( h, other ); }
	self& operator= ( const self& other ) { copy( handle, other ); }

	void clear()
	{
		this->unsubscribe();
		handle   = nullptr;
		callback = nullptr;
	}

	void bind( handle_ptr h, callback_type f ) 
	{ 
		handle   = h;
		callback = f;
	}

	inline bool is_active() const { return handle && callback && this->_is_active(); }

protected:

	void copy( handle_ptr h, const self& other ) 
	{
		if ( &other != this && h && other.is_active() )
		{
			bind( h, other.callback );
			this->_copy( &other );
		}
	}

	inline void operator() ( data_input data ) 
	{ MemberSlotCallback<handle_type,data_type>::callback(data,handle,callback); }

	callback_type callback;
	handle_ptr handle;
};

}

#endif