/******************************************************************************
 *
 * Copyright 2011-2012, Qualcomm Innovation Center, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 *****************************************************************************/

#pragma once

#include <alljoyn/MsgArg.h>
#include <qcc/String.h>
#include <map>
#include <list>
#include <qcc/ManagedObj.h>
#include <qcc/Mutex.h>

namespace AllJoyn {

/// <summary>
/// Enumeration of the various message arg types.
/// </summary>
/// <remarks>
/// Most of these map directly to the values used in the
/// DBus wire protocol but some are specific to the AllJoyn implementation.
/// </remarks>
public enum class AllJoynTypeId {
    /// <summary> AllJoyn INVALID typeId</summary>
    ALLJOYN_INVALID          = ajn::ALLJOYN_INVALID,
    /// <summary> AllJoyn array container type</summary>
    ALLJOYN_ARRAY            = ajn::ALLJOYN_ARRAY,
    /// <summary> AllJoyn boolean basic type, @c 0 is @c FALSE and @c 1 is @c TRUE - Everything else is invalid</summary>
    ALLJOYN_BOOLEAN          = ajn::ALLJOYN_BOOLEAN,
    /// <summary> AllJoyn IEEE 754 double basic type</summary>
    ALLJOYN_DOUBLE           = ajn::ALLJOYN_DOUBLE,
    /// <summary> AllJoyn dictionary or map container type - an array of key-value pairs</summary>
    ALLJOYN_DICT_ENTRY       = ajn::ALLJOYN_DICT_ENTRY,
    /// <summary> AllJoyn signature basic type</summary>
    ALLJOYN_SIGNATURE        = ajn::ALLJOYN_SIGNATURE,
    /// <summary> AllJoyn socket handle basic type</summary>
    ALLJOYN_HANDLE           = ajn::ALLJOYN_HANDLE,
    /// <summary> AllJoyn 32-bit signed integer basic type</summary>
    ALLJOYN_INT32            = ajn::ALLJOYN_INT32,
    /// <summary> AllJoyn 16-bit signed integer basic type</summary>
    ALLJOYN_INT16            = ajn::ALLJOYN_INT16,
    /// <summary> AllJoyn Name of an AllJoyn object instance basic type</summary>
    ALLJOYN_OBJECT_PATH      = ajn::ALLJOYN_OBJECT_PATH,
    /// <summary> AllJoyn 16-bit unsigned integer basic type</summary>
    ALLJOYN_UINT16           = ajn::ALLJOYN_UINT16,
    /// <summary> AllJoyn struct container type</summary>
    ALLJOYN_STRUCT           = ajn::ALLJOYN_STRUCT,
    /// <summary> AllJoyn UTF-8 NULL terminated string basic type</summary>
    ALLJOYN_STRING           = ajn::ALLJOYN_STRING,
    /// <summary> AllJoyn 64-bit unsigned integer basic type</summary>
    ALLJOYN_UINT64           = ajn::ALLJOYN_UINT64,
    /// <summary> AllJoyn 32-bit unsigned integer basic type</summary>
    ALLJOYN_UINT32           = ajn::ALLJOYN_UINT32,
    /// <summary> AllJoyn variant container type</summary>
    ALLJOYN_VARIANT          = ajn::ALLJOYN_VARIANT,
    /// <summary> AllJoyn 64-bit signed integer basic type</summary>
    ALLJOYN_INT64            = ajn::ALLJOYN_INT64,
    /// <summary> AllJoyn 8-bit unsigned integer basic type</summary>
    ALLJOYN_BYTE             = ajn::ALLJOYN_BYTE,
    /// <summary> Never actually used as a typeId: specified as ALLJOYN_STRUCT</summary>
    ALLJOYN_STRUCT_OPEN      = ajn::ALLJOYN_STRUCT_OPEN,
    /// <summary> Never actually used as a typeId: specified as ALLJOYN_STRUCT</summary>
    ALLJOYN_STRUCT_CLOSE     = ajn::ALLJOYN_STRUCT_CLOSE,
    /// <summary> Never actually used as a typeId: specified as ALLJOYN_DICT_ENTRY</summary>
    ALLJOYN_DICT_ENTRY_OPEN  = ajn::ALLJOYN_DICT_ENTRY_OPEN,
    /// <summary> Never actually used as a typeId: specified as ALLJOYN_DICT_ENTRY</summary>
    ALLJOYN_DICT_ENTRY_CLOSE = ajn::ALLJOYN_DICT_ENTRY_CLOSE,
    /// <summary> AllJoyn array of booleans</summary>
    ALLJOYN_BOOLEAN_ARRAY    = ajn::ALLJOYN_BOOLEAN_ARRAY,
    /// <summary> AllJoyn array of IEEE 754 doubles</summary>
    ALLJOYN_DOUBLE_ARRAY     = ajn::ALLJOYN_DOUBLE_ARRAY,
    /// <summary> AllJoyn array of 32-bit signed integers</summary>
    ALLJOYN_INT32_ARRAY      = ajn::ALLJOYN_INT32_ARRAY,
    /// <summary> AllJoyn array of 16-bit signed integers</summary>
    ALLJOYN_INT16_ARRAY      = ajn::ALLJOYN_INT16_ARRAY,
    /// <summary> AllJoyn array of 16-bit unsigned integers</summary>
    ALLJOYN_UINT16_ARRAY     = ajn::ALLJOYN_UINT16_ARRAY,
    /// <summary> AllJoyn array of 64-bit unsigned integers</summary>
    ALLJOYN_UINT64_ARRAY     = ajn::ALLJOYN_UINT64_ARRAY,
    /// <summary> AllJoyn array of 32-bit unsigned integers</summary>
    ALLJOYN_UINT32_ARRAY     = ajn::ALLJOYN_UINT32_ARRAY,
    /// <summary> AllJoyn array of 64-bit signed integers</summary>
    ALLJOYN_INT64_ARRAY      = ajn::ALLJOYN_INT64_ARRAY,
    /// <summary> AllJoyn array of 8-bit unsigned integers</summary>
    ALLJOYN_BYTE_ARRAY       = ajn::ALLJOYN_BYTE_ARRAY,
    /// <summary> This never appears in a signature but is used for matching arbitrary message args</summary>
    ALLJOYN_WILDCARD         = ajn::ALLJOYN_WILDCARD
};

ref class __MsgArg {
  private:
    friend ref class MsgArg;
    friend class _MsgArg;
    __MsgArg();
    ~__MsgArg();

    property Object ^ Value;
    property Object ^ Key;
};

class _MsgArg : protected ajn::MsgArg {
  protected:
    friend class qcc::ManagedObj<_MsgArg>;
    friend ref class MsgArg;
    friend class _BusObject;
    friend ref class BusObject;
    friend ref class ProxyBusObject;
    _MsgArg();
    ~_MsgArg();

    ::QStatus BuildArray(ajn::MsgArg* arry, const qcc::String elemSig, const Platform::Array<Platform::Object ^> ^ args, int32_t& argIndex);
    ::QStatus VBuildArgs(const char*& signature, uint32_t sigLen, ajn::MsgArg* arg,  int32_t maxCompleteTypes, const Platform::Array<Platform::Object ^> ^ args, int32_t& argIndex, int32_t recursionLevel);
    void SetObject(AllJoyn::MsgArg ^ msgArg, bool isKey);

    __MsgArg ^ _eventsAndProperties;
    std::map<void*, void*> _refMap;
    std::list<qcc::String> _strRef;
    std::list<void*> _msgScratch;
};

/// <summary>
/// This class deals with the message bus types and the operations on them
/// </summary>
/// <remarks>
/// MsgArgs are designed to be light-weight. A MsgArg will normally hold references to the data
/// (strings etc.) it wraps and will only copy that data if the MsgArg is assigned. For example no
/// additional memory is allocated for an #ALLJOYN_STRING that references an existing const char*.
/// If a MsgArg is assigned the destination receives a copy of the contents of the source. The
/// Stabilize() methods can also be called to explicitly force contents of the MsgArg to be copied.
/// </remarks>
public ref class MsgArg sealed {
  public:
    /// <summary>
    /// Constructor to build a message arg.
    /// </summary>
    MsgArg();

    /// <summary>
    /// Constructor to build a message arg. If the constructor fails for any reason the type will be
    /// set to #ALLJOYN_INVALID.
    /// </summary>
    /// <param name="signature">The signature for MsgArg value.</param>
    /// <param name="args">One or more values to initialize the MsgArg.</param>
    /// <remarks>
    /// - <c>'a'</c>  The array length followed by:
    ///         - If the element type is a basic type a pointer to an array of values of that type.
    ///         - If the element type is string a pointer to array of const char*, if array length is
    ///           non-zero, and the char* pointer is NULL, the NULL must be followed by a pointer to
    ///           an array of const qcc::String.
    ///         - If the element type is an @ref ALLJOYN_ARRAY "ARRAY", @ref ALLJOYN_STRUCT "STRUCT",
    ///           @ref ALLJOYN_DICT_ENTRY "DICT_ENTRY" or @ref ALLJOYN_VARIANT "VARIANT" a pointer to an
    ///           array of MsgArgs where each MsgArg has the signature specified by the element type.
    ///         - If the element type is specified using the wildcard character '*', a pointer to
    ///           an  array of MsgArgs. The array element type is determined from the type of the
    ///           first MsgArg in the array, all the elements must have the same type.
    /// - <c>'b'</c>  A bool value
    /// - <c>'d'</c>  A double (64 bits)
    /// - <c>'g'</c>  A pointer to a NUL terminated string (pointer must remain valid for lifetime of the MsgArg)
    /// - <c>'h'</c>  A qcc::SocketFd
    /// - <c>'i'</c>  An int (32 bits)
    /// - <c>'n'</c>  An int (16 bits)
    /// - <c>'o'</c>  A pointer to a NUL terminated string (pointer must remain valid for lifetime of the MsgArg)
    /// - <c>'q'</c>  A uint (16 bits)
    /// - <c>'s'</c>  A pointer to a NUL terminated string (pointer must remain valid for lifetime of the MsgArg)
    /// - <c>'t'</c>  A uint (64 bits)
    /// - <c>'u'</c>  A uint (32 bits)
    /// - <c>'v'</c>  Not allowed, the actual type must be provided.
    /// - <c>'x'</c>  An int (64 bits)
    /// - <c>'y'</c>  A byte (8 bits)
    ///
    /// - <c>'('</c> and <c>')'</c>The list of values that appear between the parentheses using the notation above
    /// - <c>'{'</c> and <c>'}'</c>A pair values using the notation above.
    ///
    /// - <c>'*'</c> A pointer to a MsgArg.
    /// </remarks>
    /// <example>
    /// An array of strings
    /// <code>
    ///    char* fruits[3] =  { "apple", "banana", "orange" };
    ///    MsgArg bowl;
    ///    bowl.Set("as", 3, fruits);
    /// </code>
    /// A struct with a uint and two string elements.
    ///
    /// <code> arg.Set("(uss)", 1024, "hello", "world");</code>
    /// An array of 3 dictionary entries where each entry has an integer key and string value.
    /// <code>
    ///    MsgArg dict[3];
    ///    dict[0].Set("{is}", 1, "red");
    ///    dict[1].Set("{is}", 2, "green");
    ///    dict[2].Set("{is}", 3, "blue");
    ///    arg.Set("a{is}", 3, dict);
    /// </code>
    /// An array of uint_16's
    /// <code>
    ///    uint16_t aq[] = { 1, 2, 3, 5, 6, 7 };
    ///    arg.Set("aq", sizeof(aq) / sizeof(uint16_t), aq);
    /// </code>
    /// </example>
    /// <exception cref="Platform::COMException">
    /// HRESULT will contain the AllJoyn error status code for the error.
    ///    - #ER_OK if the MsgArg was successfully set
    ///    - An error status otherwise
    /// </exception>
    MsgArg(Platform::String ^ signature, const Platform::Array<Platform::Object ^> ^ args);

    property Object ^ Value
    {
        Platform::Object ^ get();
    }

    property Object ^ Key
    {
        Platform::Object ^ get();
    }

    static void SetTypeCoercionMode(Platform::String ^ mode);

  private:
    friend class _MsgArg;
    friend ref class BusObject;
    friend class _BusObject;
    friend ref class ProxyBusObject;
    friend ref class Message;
    friend ref class MessageHeaderFields;
    friend class _ProxyBusObjectListener;
    MsgArg(const ajn::MsgArg * msgArg);
    MsgArg(const qcc::ManagedObj<_MsgArg>* msgArg);
    ~MsgArg();

    qcc::ManagedObj<_MsgArg>* _mMsgArg;
    _MsgArg* _msgArg;
};

}
// MsgArg.h
