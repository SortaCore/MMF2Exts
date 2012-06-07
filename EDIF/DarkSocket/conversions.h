/* Converts a number to the one's complement representation */
template<class T> inline T to_oc(T value)
{
	/* If the value is negative, we need to get the absolute value and then flip all bytes */
	if(value < 0)
	{
		return ~(-value);
	}
	/* Otherwise, it's easy. Just return the number! */
	return value;
}

/* Converts to a normal number */
template<class T> inline T from_oc(T value)
{
	/* A mask with which we can extract the highest (sign) bit */
	const T SIGN_BIT = (T)(1 << (sizeof(T)*8-1));

	/* If the sign bit is set, the number is negative and we return the complement */
	if(value & SIGN_BIT)
	{
		return -(~value & ~SIGN_BIT);
	}
	/* Otherwise, it's easy again - return the number. */
	return value;
}