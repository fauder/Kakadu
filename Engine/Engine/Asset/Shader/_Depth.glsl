#include "_Intrinsic_Other.glsl"

float LinearizeDepth( float screen_space_depth_value )
{
	/* Let's recap how View/Camera space Z value (called Vz from now on) is transformed into the Screen space Z value:
	 *	Vz [range: near,far] -> Perspective Projection -> Perspective Divide -> NDC Z (called ndc_z from now on) [range: -1, +1] -> Viewport Transformation -> Screen space Z [range: 0, +1].
	 * Undoing the Viewport transform is easy; Just a scale & translation: */

	float ndc_z = 2.0 * screen_space_depth_value - 1.0; // Undoing Viewport transformation: [0,+1] ->[-1,+1].

	/* Now we have to undo the Perspective Divide & the Perspective Projection. Focusing on the Vz [range: near,far] -> Perspective Projection -> Perspective Divide part, we see that: 
	 *	Projected_z = Vz * Pers.Proj.[2,2] + 1 * Pers.Proj.[3,2]
	 *	Projected_w = Vz
	 *
	 * Then, to convert the Projected_z to ndc_z:
	 *	ndc_z = Projected_z / Projected_w
	 * => ndc_z = ( Vz * Pers.Proj.[2,2] + Pers.Proj.[3,2] ) / Vz
	 *
	 * This brings us to the final formula for Vz:
	 * Vz = Pers.Proj.[3,2] / ( ndc_z - Pers.Proj.[2,2] ) */

	/* NOTE: Matrices are treated as row_major in the comments above, but are indexed with [column][row] below in actual shader code. */

	float view_space_z = _INTRINSIC_TRANSFORM_PROJECTION[ 2 ][ 3 ] / ( ndc_z - _INTRINSIC_TRANSFORM_PROJECTION[ 2 ][ 2 ] );

	return view_space_z;
}