
struct _lw_flashpolicy
{
	lw_server server;

	char * buffer;
	size_t size;

	lw_flashpolicy_hook_error on_error;

	void * tag;
};
// Must be declared separately, or on_error can't be linked to from C++ functions.
