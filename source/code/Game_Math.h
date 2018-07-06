#if !defined(GAME_MATH_H)

struct V2
{
	f32 X;
	f32 Y;
};

inline V2 operator-(V2 input)
{
	V2 result;
	result.X = -input.X;
	result.Y = -input.Y;

	return result;
}

inline V2 operator+(V2 input1, V2 input2)
{
	V2 result;
	result.X = input1.X + input2.X;
	result.Y = input1.Y + input2.Y;

	return result;
}

inline V2 operator+=(V2 input1, V2 input2)
{
	V2 result;

	result.X = input1.X += input2.X;
	result.Y = input1.Y += input2.Y;

	return result;
}

inline V2 operator-(V2 input1, V2 input2)
{
	V2 result;
	result.X = input1.X - input2.X;
	result.Y = input1.Y - input2.Y;

	return result;
}

inline V2 operator*(V2 input1, V2 input2)
{
	V2 result;
	result.X = input1.X * input2.X;
	result.Y = input1.Y * input2.Y;

	return result;
}

inline V2 operator*=(V2 input1, V2 input2)
{
	V2 result;
	result.X = input1.X * (input1.X * input2.X);
	result.Y = input1.Y * (input1.Y * input2.Y);

	return result;
}

inline V2 operator*=(V2 vector, f32 scalar)
{
	V2 result;
	result.X = vector.X * (vector.X * scalar);
	result.Y = vector.Y * (vector.Y * scalar);

	return result;
}

inline V2 operator*(V2 vector, f32 scalar)
{
	V2 result;
	result.X = vector.X * scalar;
	result.Y = vector.Y * scalar;

	return result;
}

inline V2 operator*(f32 scalar, V2 vector)
{
	V2 result;
	result.X = vector.X * scalar;
	result.Y = vector.Y * scalar;

	return result;
}

inline V2 operator/(V2 input1, V2 input2)
{
	V2 result;
	result.X = input1.X / input2.Y;
	result.Y = input1.Y / input2.Y;

	return result;
}

inline V2 operator/(V2 vector, f32 scalar)
{
	V2 result;
	result.X = vector.X / scalar;
	result.Y = vector.Y / scalar;

	return result;
}

inline V2 operator/=(V2 input1, V2 input2)
{
	V2 result;
	result.X = input1.X / (input1.X / input2.Y);
	result.Y = input1.Y / (input1.Y / input2.Y);

	return result;
}

inline f32 Abs(f32 input)
{
	if(input < 0 )
	{
		return input * -1;
	}
	else
	{
		return input;
	}
}

inline f32 Square(f32 input)
{
	return (input * input);
}

inline f32 Dot(V2 a, V2 b)
{
	return ((a.X*b.X) + (a.Y*b.Y));
}

//Should be intrinsic
inline f32 Clamp(f32 a, f32 min, f32 max)
{
	f32 result = a < min ? min : a;
	return result > max ? max : result;
}

#define GAME_MATH_H

#endif