/*
Copyright (C) 2008-2010 Electronic Arts, Inc.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1.  Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2.  Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3.  Neither the name of Electronic Arts, Inc. ("EA") nor the names of
its contributors may be used to endorse or promote products derived
from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ELECTRONIC ARTS AND ITS CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ELECTRONIC ARTS OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

///////////////////////////////////////////////////////////////////////////////
// EAWebKitJavascriptBinding.h
//
// By Chris Stott
///////////////////////////////////////////////////////////////////////////////

#ifndef EAWEBKIT_EAWEBKITJAVASCRIPTBINDING_H
#define EAWEBKIT_EAWEBKITJAVASCRIPTBINDING_H

#include <EAWebKit/EAWebKitJavascriptValue.h>
#include <EAWebKit/internal/EAWebKitEASTLHelpers.h> // For fixed string
#include <EAWebKit/internal/EAWebKitAssert.h>  

#include "BAL/OWBAL/Concretizations/Types/WK/BCbalValuePrivateWK.h" // For setting the JS ExecState

      


#if EAWEBKIT_THROW_BUILD_ERROR
#error This file should be included only in a dll build
#endif

namespace EA
{
	namespace WebKit
	{
		
		//////////////////////////////////////////////////////////////////////////
		//
		class JavascriptBindingObject : public BalObject
		{
		public:
			//////////////////////////////////////////////////////////////////////////
			//
            JavascriptBindingObject(EA::WebKit::View *pView = NULL)
                : mpView(pView)
            {
			}

			//////////////////////////////////////////////////////////////////////////
			//
			~JavascriptBindingObject()
			{
			}

			//////////////////////////////////////////////////////////////////////////
			//
            BalValue* invoke(const char *name, Vector<BalValue*> args, KJS::ExecState* exec )
			{

    			EA::WebKit::ViewNotification* pVN = EA::WebKit::GetViewNotification();
                if (pVN)
				{
                    JavascriptMethodInvokedInfo info;
                    info.mpView = mpView;
                    info.mMethodName = name;
                    
					unsigned counter = 0;
                    
					for (Vector<BalValue*>::const_iterator i = args.begin(); i != args.end(); ++i)
					{
                        Translate(*i, info.mArguments[counter++]);
					}
					info.mArgumentCount = counter;

					pVN->JavascriptMethodInvoked(info);

                	// CSidhall 3/25/10 -We need to set the exec in the private BalValuePrivate 
                    // so that the translate can call the exec to allocated a string when a string is returned.
                    if(mValue.d)
                        mValue.d->setExec(exec);

					return Translate(info.mReturn, mValue);
				}
				else
				{
					BalValue* value = &mValue;
					value->balUndefined();
					return value;
				}
				
			}

			//////////////////////////////////////////////////////////////////////////
			//
			BalValue *getProperty(const char *name, KJS::ExecState* exec)
			{
                BalValue* translated = new BalValue;    // The BAL is expecting the delete the returned BalValue so we create one to make it happy.
                EAW_ASSERT(translated);
                if(translated->d)
                    translated->d->setExec(exec);                        

                EA::WebKit::ViewNotification* pVN = EA::WebKit::GetViewNotification();
                if (pVN)
				{
					JavascriptPropertyInfo info;
					info.mpView = mpView;
                    info.mPropertyName = name;
					pVN->GetJavascriptProperty(info);   // On the client side, we flip it so that a get is a set. 
					Translate(info.mValue, *translated);
				}
				else
				{
					translated->balUndefined();
				}
                return translated;
            };

			//////////////////////////////////////////////////////////////////////////
			//
			void setProperty( const char *name, BalValue *value) 
			{
				EA::WebKit::ViewNotification* pVN = EA::WebKit::GetViewNotification();
                if (pVN)
				{
					JavascriptPropertyInfo info;
                    info.mpView = mpView;				
                    info.mPropertyName = name;
					Translate(value, info.mValue);
					pVN->SetJavascriptProperty(info); // On the client side, we flip it so that a set is a get.
				}
			};

		private:
			//////////////////////////////////////////////////////////////////////////
			// object marshalling 
			BalValue* Translate(JavascriptValue& value, BalValue& translated)
			{
				switch (value.GetType())
				{
                case JavascriptValueType_Undefined:
					translated.balUndefined();
					break;	
				case JavascriptValueType_Boolean:
					translated.balBoolean(value.GetBooleanValue());
					break;
				case JavascriptValueType_Number:
					translated.balNumber(value.GetNumberValue());
					break;
				case JavascriptValueType_String:
                    {
                        wchar_t* pVal = (wchar_t*) GetFixedString(value.GetStringValue())->c_str();
                        translated.balString(pVal);
                    }
                    break;
                default:
                      EAW_ASSERT(0);
                    break;
				}

				return &translated;
			}

			//////////////////////////////////////////////////////////////////////////
			//
			JavascriptValue* Translate(const BalValue* value, JavascriptValue& translated)
			{
				switch (value->type())
				{
                case KJS::BooleanType:
					translated.SetBooleanValue(value->toBoolean());
					break;
				case KJS::UndefinedType:
					translated.SetUndefined();
					break;
				case KJS::NumberType:
					translated.SetNumberValue(value->toNumber());
					break;
				case KJS::StringType:
                    {
                        mString = value->toString();
					    GetFixedString(translated.GetStringValue())->assign(mString.charactersWithNullTermination());
                        translated.SetStringType();
                    }
					break;
				default:
					translated.SetUndefined();  // Bal has some other types like null or unspecified
					break;
				}

				return &translated;
			}

            EA::WebKit::View*   mpView; // 3/17/10 CSidhall - Added for multiview support in notifications
			BalValue			mValue;
			WebCore::String		mString;
		};

	} // namespace WebKit
} // namespace EA

#endif //EAWEBKIT_EAWEBKITJAVASCRIPTBINDING_H
