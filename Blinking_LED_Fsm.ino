/*

Module: Blinking_LED_fsm.ino

Function:
	Example of how to build a simple FSM.

Copyright notice and License:
	See LICENSE file accompanying this project at
	https:://github.com/mcci-catena/catena-arduino-platform/

Author:
	Rama subbu, MCCI Corporation  June 2021

*/

#include <Catena.h>
#include <Catena_FSM.h>
#include <Catena_Led.h>

// save lots of typing by using the McciCatena namespace.
using namespace McciCatena;

/****************************************************************************\
|
|   The led class and FSM. Refer to the overall README.md for a
|   description of the FSM.
|
\****************************************************************************/

class led
	{
public:
	// constructor
	led(Catena &myCatena, StatusLed &lockLed)
		: m_Catena(myCatena),
		 m_Led(lockLed)
		{}

	// states for FSM
	enum class State
		{
		stNoChange = 0, // this name must be present: indicates "no change of state"
		stInitial,		// this name must be presnt: it's the starting state.
		stLedOn,
		stLedOff,
		stFinal, 		// this name must be present, it's the terminal state.
		};

	// the begin method initializes the fsm
	void begin()
		{
		if (! this->m_fRunning)
			this->m_fsm.init(*this, &led::fsmDispatch);
		else
			this->m_Catena.SafePrintf("already running!\n");
		}

	// the end method shuts it down
	void end()
		{
		if (this->checkRunning())
			{
			this->m_evShutdown = true;
			while (this->m_fRunning)
				this->m_fsm.eval();
			}
		}

private:
	// the FSM instance
	cFSM<led, State> m_fsm;

	// verify that FSM is running, and print a message if not.
	bool checkRunning() const
		{
		if (this->m_fRunning)
			return true;
		else
			{
			this->m_Catena.SafePrintf("not running!\n");
			return false;
			}
		}

	// the FSM dispatch function called by this->m_fsm.
	// this example is here, so we write it here, but in
	// any more complex application you'd have this outside
	// the Class definition, possibly compiled separately.
	 State fsmDispatch(State currentState, bool fEntry)
		{
		State newState = State::stNoChange;

		switch (currentState)
			{

		case State::stInitial:
			if (fEntry)
				{
				Serial.begin(9600);
				}
			this->m_fRunning = true;
			newState = State::stLedOn;
			break;

		case State::stLedOn:
			if (fEntry)		
				{
				pinMode(13, OUTPUT);
				digitalWrite(13, HIGH);
				delay(1000);	
				}
			break;

		case State::stLedOff:
			if (fEntry)
				{
				digitalWrite(13, LOW);
								delay(1000);
				}

		case State::stFinal:
			// This is called just once; we just clear the
			// running flag.  The core FSM is responsible for
			// determining if we're locked or unlocked on
			// exit. Since we always get here via the stLocked
			// state, we will, in fact, be locked.
			if (fEntry)
				{
				m_Catena.SafePrintf("led stopped!\n");
				this->m_fRunning = false;
				}
			else
				{
				m_Catena.SafePrintf("stFinal but not fEntry shouldn't happen.\n");
				}

			// stay in this state.
			break;
			}

		return newState;
		}
	}

/****************************************************************************\
|
|   The variables
|
\****************************************************************************/

// instantiate the global object for the platform.
Catena gCatena;

// instantiate the LED object
StatusLed gLed (Catena::PIN_STATUS_LED);

// instantiate the led
led gled (gCatena, gLed);

/*
|| The next few lines give the datastructures needed for extending
|| the command parser.
*/

/****************************************************************************\
|
|   The code
|
\****************************************************************************/

// setup is called once.
void setup()
	{
	gCatena.begin();

	/* wait 2 seconds */
	auto const now = millis();
	while (millis() - now < 2000)
		/* wait */;

	/* wait for a UART connection */
	while (! Serial)
	/* wait */;

	/* add our application-specific commands */
	gCatena.addCommands(
		/* name of app dispatch table, passed by reference */
		sMyExtraCommands_top,
		/*
		|| optionally a context pointer using static_cast<void *>().
		|| normally only libraries (needing to be reentrant) need
		|| to use the context pointer.
		*/
		nullptr
		);


	gCatena.SafePrintf("This is the FSM demo program for the MCCI Catena-Arduino-Platform library.\n");
	gCatena.SafePrintf("Enter 'help' for a list of commands.\n");
	gCatena.SafePrintf("(remember to select 'Line Ending: Newline' at the bottom of the monitor window.)\n");

	gLed.begin();
	gCatena.registerObject(&gLed);
	gled.begin();
	}

// loop is called repeatedly.
void loop()
	{
	// Tn this app, all we have to do is invoke the Catena
	// polling framework. Everytng wlse will be driven from that.
	gCatena.poll();
	}
	