Serializer
=============


The serializer is a central component in all arpc implementations. When serializing a function call,
the high level API passes the function name and the parameters to the serializer, which generates a
serialized representation of said function call. Likewise, when the high level API receives such a
representation, it passes it to the serializer, which deserializes the function name and the parameters.
The high level API then uses these to invoke the corresponding registered function.
The serializer is also a container for the function name and parameters; prior to the actual serialization,
the user is required to use the accessor functions to store these values in the serializer. When using the
serializer to deserialize a function call, the deserialized function name and parameters are stored in the
serializer as well, thus after deserializing, the user can retrieve these values using the accessors.

The user never directly calls these functions; they are always called by the high level API.


Functions
---------

The serializer must implement two functions:

- `serializer.read_from(serialized_representation)`
  Deserializes a function call from the given serialized representation. It stores the deserialized
  function name and parameters inside the serializer. This function does not return a value.

- `serializer.write_to()`
  Creates a serialized representation out of the function name and parameters previously stored in the
  serializer, and returns the serialized representation.

- `set_function_name(function_name)/get_function_name()`
  Accessors for the function name.

- `set_parameters(parameters)/get_parameters()`
  Accessors for the parameters.


Serialized representation
-------------------------

The serialized representation is simply a sequence of bytes. In most scripting languages, a string type
is suited for storing the representation. *However*, it is necessary to ensure that it can store binary
data properly. In general, serializers are not required to make sure the representation is valid ascii text;
some serializers (such as JSON serializers) may do so, but there is no guarantee.

The references for each implementation go into further detail about this topic.

