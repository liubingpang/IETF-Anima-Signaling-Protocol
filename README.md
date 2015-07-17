API for GDNP-draft03

Server.h

ERROR server_init() 
Intiation of GDNP server, and start to listen for discovery.

ERROR listen_negotiate()
Start to listen for negotiation and synchronization.
 
ERROR stop_negotiate() 
Stop listening.

virtual bool asa_geq_fn(const void * value_a, const void * value_b) 
Provided by the ASA for GDNP to pass the negotiated value to ASA and return the value for negotiation, should be overwritten.

virtual void * asa_negotiate_result(const void * value) 
Provided by the ASA for comparing whether the value is equal, should be overwritten.


Client.h

ERRNO discover()
Start to discover.

ERRNO negotiate(const void* buffer_obj)
Start to negotiate.

ERRNO synchronize(const void* buffer_obj)
Start to synchronize. 

virtual bool asa_geq_fn(const void * value_a, const void * value_b) 
Provided by the ASA for GDNP to pass the negotiated value to ASA and return the value for negotiation, should be overwritten.

virtual void * asa_negotiate_result(const void * value) 
Provided by the ASA for comparing whether the value is equal, should be overwritten.

virtual void do_configuration(const void * nego_result)
Provided by the ASA for GDNP to do configuration by using negotiation result, should be overwritten.

