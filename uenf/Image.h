// design rationale: now COW-Idiom here, is too complicated to get right and might lead to unexpected behaviour (that is, it
// can cost the original owner(-thread) of the image cpu time in case he is first to modifiy the image and then needs to 
// copy it. Also, a user would have to declare the copied image constant to avoid copies, if an access operator is invoked 
// (even only for read access)
