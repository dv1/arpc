arpc - asynchronous, I/O agnostic RPC library
=============================================


Introduction
------------

arpc is a small library for simple, asynchronous RPC calls.
An asynchronous call is one where the caller does not wait for the called remote function
to finish execution, implying that the caller does not care about the function's return value.

This makes asynchronous RPC calls akin to a little more constrained form of message passing,
where the message contents are not free-form, but structured; each message contains the parameters
for the function call and the name of the called function.

arpc can also be viewed as a library for (de)serializing function calls. Serialization is an
integral part of arpc; the function call is serialized to a message that can be stored and/or
transmitted, and deserialized by the receiver. The deserialized call can then be used for invoking
a function.

The actual transport of the serialized call is _not_ part of arpc. Once the function call is serialized,
it is up to the programmer to do something with that result. Said result is just a sequence of bytes.
It can be transmitted, stored, in any way. arpc does not enforce or include specific transport and/or
storage mechanisms. It also does not contain error correction; it is up to the transport/storage
mechanism to ensure data integrity.


Motivation
----------

After several projects where message passing became a fundamental part of the architecture, it became fully
clear that a library for generating, and parsing messages would be useful, given that these are tedious
and error-prone tasks.

Since messages can be viewed as function calls where the caller does not wait for
the function to finish executing, and does not care about the function's return value, the idea came up to
write a library that generates messages out of function calls, and invokes registered functions by incoming
messages.

In addition, there should be implementations for various languages, allowing for function
calls between processes written in different languages without the need for some glue layer.


Overview
--------

arpc is made up of two main components:

- the high level API, simply called "arpc API": provides function for registering functions that can be invoked
  by incoming messages, and for serializing/deserializing/invoking function calls.
- the serializer: while the arpc API transforms function calls into a string (the function name) and
  a sequence of values (the parameters), the serializer handles the actual serialization; it receives
  said function name and sequence of parameter values, and produces a serialized result.
  The serializer also contains facilities for deserializing; in that case, it returns a function name and
  a sequence of parameter values.

From the outside, only the arpc API is of interest; the serializer type is passed to the arpc API at time of
initialization, and is not interacted with directly.

A minimal working setup involves these steps:

1. Initialize arpc, with a chosen serializer
2. Register at least one function, otherwise nothing will get called by incoming messages
3. Perform a function call using arpc, producing a message
4. Feed that message into arpc, causing the called function to be invoked

Typically, step 3 is done by a sender, while steps 2 and 4 are done by the receiver.
(Step 1 is done by both, and both have to agree on the same serializer type.)


A simple Ruby example:

	require 'arpc/arpc'
	require 'arpc/json_serializer'

	arpc1 = ARPC::ARPC.new ARPC::JsonSerializer
	serialized_call = arpc1.serialize.foo "hello", 3.14

	def foo(a,b)
		puts "foo #{a} #{b}"
	end

	arpc2 = ARPC::ARPC.new ARPC::JsonSerializer
	arpc2.register_function :foo
	arpc2.invoke_serialized_call serialized_call

This examples uses two instances, arpc1 and arpc2, to provide a better example. Typically, sender (arpc1)
and receiver (arpc2) are two separate processes/threads/nodes. In theory, one could serialize a call
with an arpc instance and feed the serialized call back to this instance.

The example above does the following steps:

1. Create arpc1, using the JSON serializer
2. Serialize a function call: calling a "foo" function, with parameters "hello" and 3.14
3. Create arpc2, using the JSON serializer
4. Register the foo function with arpc2
5. Pass the serialized call from step 2 into arpc2, causing it to call foo with the parameters from step 2
6. "hello 3.14" is printed

This example uses the JSON serializer. The serialized function call then looks like this JSON object:

	{
		"func" : "foo",
		"params" : ["hello", 3.14]
	}

__NOTE:__ Further documentation (reference, guide for extending arpc) is currently being written. When finished,
it will reside in a docs/ folder.


Future work & ideas
-------------------

Note that arpc is _not_ tightly coupled with JSON. JSON is the currently existing serializer. In future,
BSON will be added as well. XML is also a possibility, so is YAML. Google Protocol Buffers could be used,
but are problematic, since they expect the structure of the data to be known in advance.

Likewise, arpc is not a Ruby-only project. Currently, implementations for Ruby, C++, and JavaScript exist.
More are planned (Python, Scala, Java, C#, amongst others).

An intriguing idea is to use arpc together with [ZeroMQ](http://www.zeromq.org/). The serialized function call
is just a bunch of bytes, which fits perfectly with the ZeroMQ `recv()` and `send()` functions. Also,
since in arpc, the sender does not wait for the receiving function to finish executing, multicast can be
trivially used with arpc, issuing one function call to N receivers.
