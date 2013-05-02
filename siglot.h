#ifndef __SIGLOT__
#define __SIGLOT__

#include <set>

//=============================================
// @filename     siglot.h
// @date         May 2nd 2013
// @author       Sheljohn (Jonathan H)
// @contact      sh3ljohn+siglot [at] gmail
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
	inline void disconnect() { connected = false; }

	// Trigger the callback function
	virtual void operator() ( const data_type& data ) =0;
};



/**
 * The Signal interface as seen by a Slot object.
 * 
 * A Signal stores its listeners (Slots) as a set of pointers to 
 * CallbackInterfaces. This allows to avoid duplicates, and provides 
 * an efficient way of "detaching" a specific Slot.
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

	inline unsigned count() const { return slots.size(); }

protected:

	template <typename U> friend class ListenerInterface;

	std::set<slot_ptr> slots;
	inline void attach( slot_ptr s ) { slots.insert(s); }
	inline void detach( slot_ptr s ) { slots.erase(s); }
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

	void detach()
		{
			if ( this->connected && signal ) signal->detach(this);
			this->disconnect();
			signal = nullptr;
		}

	void listen_to( signal_ptr s )
		{
			signal = s;
			s->attach(this);
			this->connected = true;
		}

protected:

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
struct Signal : public SlotSet<data_type>
{
	data_type data;

	~Signal() { clear(); }

	void clear()
		{ 
			for ( auto slot : this->slots ) slot->disconnect();
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

	typedef const data_type& data_input;
	typedef void (*callback_type)( data_input );

	Slot() { clear(); }
	Slot( callback_type f ) { clear(); bind(f); }
	~Slot() { clear(); }

	inline void bind( callback_type f ) { callback = f; }
	inline void clear() { this->detach(); }
	inline bool is_active() const { return this->connected && this->signal; }

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

	typedef const VoidData& data_input;
	typedef void (*callback_type)();

	Slot() { clear(); }
	Slot( callback_type f ) { clear(); bind(f); }
	~Slot() { clear(); }

	inline void bind( callback_type f ) { callback = f; }
	inline void clear() { this->detach(); }
	inline bool is_active() const { return this->connected && this->signal; }

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

	typedef handle_type* handle_ptr;
	typedef const data_type& data_input;
	typedef void (handle_type::*callback_type)( data_input );

	MemberSlot() { clear(); }
	MemberSlot( handle_ptr h, callback_type f ) { clear(); bind(h,f); }
	~MemberSlot() { clear(); }

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

	inline bool is_active() const { return handle && this->connected && this->signal; }

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

	typedef handle_type* handle_ptr;
	typedef const VoidData& data_input;
	typedef void (handle_type::*callback_type)();

	MemberSlot() { clear(); }
	MemberSlot( handle_ptr h, callback_type f ) { clear(); bind(h,f); }
	~MemberSlot() { clear(); }

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

	inline bool is_active() const { return handle && this->connected && this->signal; }

protected:

	inline void operator() ( data_input data ) { (handle->*callback)(); }
	callback_type callback;
	handle_ptr handle;
};

}

#endif