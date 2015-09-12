/*
 *  MessageInflater.cpp
 *  Created by Woody Zenfell, III on Thu Sep 11 2003.
 */

/*
  Copyright (c) 2003, Woody Zenfell, III

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#if !defined(DISABLE_NETWORKING)

#include "MessageInflater.h"
#include "Logging.h"

Message*
MessageInflater::inflate(const UninflatedMessage& inSource)
{
	Message* theResult = NULL;
	
	MessageInflaterMap::iterator i = mMap.find(inSource.inflatedType());
	if(i != mMap.end())
	{
		try
		{
			theResult = i->second->clone();
			if(theResult != NULL)
			{
				bool successfulInflate = theResult->inflateFrom(inSource);
				if(!successfulInflate)
				{
					logWarning("inflate failed of message type %i", inSource.inflatedType());
					throw 1;
				}
			} else {
				logWarning("clone() failed message type %i", inSource.inflatedType());
			}
		}
		catch(...)
		{
			logWarning("exception caught in inflated() message type %i", inSource.inflatedType());
			delete theResult;
			theResult = NULL;
		}
	} else {
		logAnomaly("do not know how to inflate message type %i", inSource.inflatedType());
	}

	if(theResult == NULL)
	{
		// We should end up here in any of the following circumstances:
		// 1. No entry in map
		// 2. clone() returned NULL
		// 3. exception thrown in clone()
		// 4. inflateFrom() returned "false" meaning "unsuccessful inflate"
		// 5. exception thrown in inflateFrom()

		// In any of these cases, we were unable to inflate the message, so we return
		// a new copy of the uninflated message.
		theResult = inSource.clone();
	}

	return theResult;
}



void
MessageInflater::learnPrototypeForType(MessageTypeID inType, const Message& inPrototype)
{
	Message* theClone = inPrototype.clone();

	Message* theExistingPrototype = mMap[inType];
	delete theExistingPrototype;
	
	mMap[inType] = theClone;
}



void
MessageInflater::removePrototypeForType(MessageTypeID inType)
{
	MessageInflaterMap::iterator i = mMap.find(inType);
	if(i != mMap.end())
	{
		delete i->second;
		mMap.erase(i);
	}
}



MessageInflater::~MessageInflater()
{
	for(MessageInflaterMap::iterator i = mMap.begin(); i != mMap.end(); i++)
	{
		delete i->second;
	}
}

#endif // !defined(DISABLE_NETWORKING)
