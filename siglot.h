#ifndef __SIGLOT__
#define __SIGLOT__

#include <set>

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

	CallbackInterface(): connected(false) {}

protected:

	template <typename U> friend class Signal;

	// Tells if this Callback is connected to a Signal
	bool connected;

	// Used by a Signal at its destruction to notify Slots
	inline void _disconnect() { connected = false; }

	// Trigger the callback function
	virtual void operator() ( const data_type& data ) =0;
};



/**
 * The Signal interface as seen by a Slot object.
 * 
 * A Signal stores its listeners (Slots) as a set of pointers to 
 * CallbackInterfaces. This allows to avoid duplicates, and to access 
 * specific Slot in logarithmic time.
 *
 * This interface provides the methods:
 * - attach: a new Slot is listening to this Signal
 * - detach: a specific Slot goes off
 * - count : returns the number of currently attached Slots
 */
template <typename data_type>
class SlotSet
{
public:

	typedef CallbackInterface<data_type> slot_type;
	typedef slot_type* slot_ptr;
	typedef SlotSet<data_type> self;

	inline unsigned count() const { return slots.size(); }

protected:

	template <typename U> friend class ListenerInterface;

	inline void _copy( const self *other )
	{
		if ( other != this ) slots = other->slots;
	}

	std::set<slot_ptr> slots;
	inline void _attach( slot_ptr s ) { slots.insert(s); }
	inline void _detach( slot_ptr s ) { slots.erase(s); }
};



/**
 * This interface defines the Slot-actions which are essentially related 
 * to a Signal:
 * - detach   : breaks the connection Slot-Signal
 * - listen_to: attach this Slot to the input Signal
 */
template <typename data_type>
class ListenerInterface : public CallbackInterface<data_type>
{
public:

	typedef SlotSet<data_type> signal_type;
	typedef signal_type* signal_ptr;
	typedef ListenerInterface<data_type> self;

	void detach()
	{
		if ( this->connected && signal ) signal->_detach(this);
		this->_disconnect();
		signal = nullptr;
	}

	void listen_to( signal_ptr s )
	{
		signal = s;
		s->_attach(this);
		this->connected = true;
	}

	inline virtual bool is_active() { return this->connected = _is_active(); }
	inline bool _is_active() const { return this->connected && signal; }

protected:

	void copy( const self *other )
	{
		if ( other != this && other->_is_active() )
			listen_to( other->signal );
	}

	signal_ptr signal;
};



	/********************     **********     ********************/
	/********************     **********     ********************/



/**
 * The Signal class to use in your code.
 * The template argument corresponds to the type of the "event data structure".
 * The method "invoke" triggers the callback functions of all attached Slots.
 */
template <typename data_type = VoidData>
class Signal : public SlotSet<data_type>
{
public:

	typedef Signal<data_type> self;

	data_type data;

	Signal() {}
	~Signal() { clear(); }

	Signal( const self& other ) {}
	self& operator= ( const self& other ) {}

	inline void copy( const self& other )
	{
		this->_copy( &other );
	}

	void clear()
	{ 
		for ( auto slot : this->slots ) slot->_disconnect();
		this->slots.clear();
	}

	void invoke() const
	{
		for ( auto slot : this->slots ) (*slot)(data);
	}
};



/**
 * The Slot class is to be used with non-member functions callbacks:
 * - bind     : bind the slot to a callback function
 * - is_active: tells if the Slot is connected to a Signal
 *
 * NOTE:
 * The callback function must have the following signature
 * void callback_function( const data_type& data )
 */
template <typename data_type = VoidData>
class Slot : public ListenerInterface<data_type>
{
public:

	typedef Slot<data_type> self;
	typedef const data_type& data_input;
	typedef void (*callback_type)( data_input );

	Slot() { clear(); }
	Slot( callback_type f ) { clear(); bind(f); }
	~Slot() { clear(); }

	Slot( const self& other ) { this->copy( &other ); }
	self& operator= ( const self& other ) { this->copy( &other ); }

	inline void bind( callback_type f ) { callback = f; }
	inline void clear() { this->detach(); }

protected:

	inline void operator() ( data_input data ) { callback(data); }
	callback_type callback;
};



/**
 * Template specialization for Signals without data.
 * To be declared as follows: Slot<> s( callback_function );
 *
 * NOTE:
 * In this case, the callback function must have the following signature:
 * void callback_function()
 */
template <>
class Slot<VoidData> : public ListenerInterface<VoidData>
{
public:

	typedef Slot<VoidData> self;
	typedef const VoidData& data_input;
	typedef void (*callback_type)();

	Slot() { clear(); }
	Slot( callback_type f ) { clear(); bind(f); }
	~Slot() { clear(); }

	Slot( const self& other ) { this->copy( &other ); }
	self& operator= ( const self& other ) { this->copy( &other ); }

	inline void bind( callback_type f ) { callback = f; }
	inline void clear() { this->detach(); }

protected:

	inline void operator() ( data_input data ) { callback(); }
	callback_type callback;
};



/**
 * The MemberSlot class is to be used with member-function callbacks:
 * - bind     : bind the slot to the current instance and the member callback function
 * - is_active: tells if the Slot is connected to a Signal
 *
 * NOTE: The binding function is used as follows:
 * MemberSlot<handle_type,data_type> slot;
 * slot.bind(this, &handle_type::callback_function);
 */
template <typename handle_type, typename data_type = VoidData>
class MemberSlot : public ListenerInterface<data_type>
{
public:

	typedef MemberSlot<handle_type,data_type> self;
	typedef handle_type* handle_ptr;
	typedef const data_type& data_input;
	typedef void (handle_type::*callback_type)( data_input );

	MemberSlot() { clear(); }
	MemberSlot( handle_ptr h, callback_type f ) { clear(); bind(h,f); }
	~MemberSlot() { clear(); }

	MemberSlot( const self& other ) { this->copy( &other ); }
	self& operator= ( const self& other ) { this->copy( &other ); }

	void clear()
	{
		this->detach();
		handle = nullptr;
	}

	void bind( handle_ptr h, callback_type f ) 
	{ 
		handle   = h;
		callback = f;
	}

	inline bool is_active() { return handle && (this->connected = this->_is_active()); }

protected:

	inline void operator() ( data_input data ) { (handle->*callback)(data); }
	callback_type callback;
	handle_ptr handle;
};



/**
 * Partial Template Specialization for Signals without data.
 * To be declared by omitting the second template parameter.
 *
 * NOTE: Same remark as before for the member callback function signature.
 */
template <typename handle_type>
class MemberSlot<handle_type,VoidData> : public ListenerInterface<VoidData>
{
public:

	typedef MemberSlot<handle_type,VoidData> self;
	typedef handle_type* handle_ptr;
	typedef const VoidData& data_input;
	typedef void (handle_type::*callback_type)();

	MemberSlot() { clear(); }
	MemberSlot( handle_ptr h, callback_type f ) { clear(); bind(h,f); }
	~MemberSlot() { clear(); }

	MemberSlot( const self& other ) { this->copy( &other ); }
	self& operator= ( const self& other ) { this->copy( &other ); }

	void clear()
		{
			this->detach();
			handle = nullptr;
		}

	void bind( handle_ptr h, callback_type f ) 
		{ 
			handle   = h;
			callback = f;
		}

	inline bool is_active() { return handle && (this->connected = this->_is_active()); }

protected:

	inline void operator() ( data_input data ) { (handle->*callback)(); }
	callback_type callback;
	handle_ptr handle;
};

}

#endif