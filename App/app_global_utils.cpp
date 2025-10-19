/*
* Main State Machine
* RAM variables (in a big union) 
* Common Functions
*/


#include <stdint.h>
#include "App_Shapes.h" // for buffer sizes
namespace app_global_utils
{

	/* The resolution of the arctan argument in bits. The arctan is only used in
	  the range which corresponds to 45 degrees.
	  Output angle is in 16 bits */
#define ARG_RESOLUTION_HI    9
	  /* ATAN table with 13 bit resolution, full circle in 16 bit resolution
	  * Table values are Theta = (16384/45)*ATAN(index/512)
	  * Table range is 45 degrees;  Correct octant to be resolved from input parameters.
	  */
	int16_t ATAN_TABLE[(1 << ARG_RESOLUTION_HI) + 1] =
	{
		0,  20,  41,  61,  81, 102, 122, 143, 163, 183, 204, 224, 244, 265, 285, 305,
		326, 346, 367, 387, 407, 428, 448, 468, 489, 509, 529, 550, 570, 590, 610, 631,
		651, 671, 692, 712, 732, 752, 773, 793, 813, 833, 854, 874, 894, 914, 935, 955,
		975, 995, 1015, 1036, 1056, 1076, 1096, 1116, 1136, 1156, 1177, 1197, 1217, 1237, 1257, 1277,
		1297, 1317, 1337, 1357, 1377, 1397, 1417, 1437, 1457, 1477, 1497, 1517, 1537, 1557, 1577, 1597,
		1617, 1637, 1656, 1676, 1696, 1716, 1736, 1756, 1775, 1795, 1815, 1835, 1854, 1874, 1894, 1914,
		1933, 1953, 1973, 1992, 2012, 2031, 2051, 2071, 2090, 2110, 2129, 2149, 2168, 2188, 2207, 2227,
		2246, 2266, 2285, 2305, 2324, 2343, 2363, 2382, 2401, 2421, 2440, 2459, 2478, 2498, 2517, 2536,
		2555, 2574, 2594, 2613, 2632, 2651, 2670, 2689, 2708, 2727, 2746, 2765, 2784, 2803, 2822, 2841,
		2860, 2879, 2897, 2916, 2935, 2954, 2973, 2991, 3010, 3029, 3047, 3066, 3085, 3103, 3122, 3141,
		3159, 3178, 3196, 3215, 3233, 3252, 3270, 3289, 3307, 3325, 3344, 3362, 3380, 3399, 3417, 3435,
		3453, 3472, 3490, 3508, 3526, 3544, 3562, 3580, 3599, 3617, 3635, 3653, 3670, 3688, 3706, 3724,
		3742, 3760, 3778, 3796, 3813, 3831, 3849, 3867, 3884, 3902, 3920, 3937, 3955, 3972, 3990, 4007,
		4025, 4042, 4060, 4077, 4095, 4112, 4129, 4147, 4164, 4181, 4199, 4216, 4233, 4250, 4267, 4284,
		4302, 4319, 4336, 4353, 4370, 4387, 4404, 4421, 4438, 4454, 4471, 4488, 4505, 4522, 4539, 4555,
		4572, 4589, 4605, 4622, 4639, 4655, 4672, 4688, 4705, 4721, 4738, 4754, 4771, 4787, 4803, 4820,
		4836, 4852, 4869, 4885, 4901, 4917, 4933, 4949, 4966, 4982, 4998, 5014, 5030, 5046, 5062, 5078,
		5094, 5109, 5125, 5141, 5157, 5173, 5188, 5204, 5220, 5235, 5251, 5267, 5282, 5298, 5313, 5329,
		5344, 5360, 5375, 5391, 5406, 5421, 5437, 5452, 5467, 5483, 5498, 5513, 5528, 5543, 5559, 5574,
		5589, 5604, 5619, 5634, 5649, 5664, 5679, 5694, 5708, 5723, 5738, 5753, 5768, 5782, 5797, 5812,
		5826, 5841, 5856, 5870, 5885, 5899, 5914, 5928, 5943, 5957, 5972, 5986, 6000, 6015, 6029, 6043,
		6058, 6072, 6086, 6100, 6114, 6128, 6142, 6157, 6171, 6185, 6199, 6213, 6227, 6240, 6254, 6268,
		6282, 6296, 6310, 6323, 6337, 6351, 6365, 6378, 6392, 6406, 6419, 6433, 6446, 6460, 6473, 6487,
		6500, 6514, 6527, 6540, 6554, 6567, 6580, 6594, 6607, 6620, 6633, 6646, 6660, 6673, 6686, 6699,
		6712, 6725, 6738, 6751, 6764, 6777, 6790, 6803, 6815, 6828, 6841, 6854, 6867, 6879, 6892, 6905,
		6917, 6930, 6943, 6955, 6968, 6980, 6993, 7005, 7018, 7030, 7043, 7055, 7068, 7080, 7092, 7105,
		7117, 7129, 7141, 7154, 7166, 7178, 7190, 7202, 7214, 7226, 7238, 7250, 7262, 7274, 7286, 7298,
		7310, 7322, 7334, 7346, 7358, 7369, 7381, 7393, 7405, 7416, 7428, 7440, 7451, 7463, 7475, 7486,
		7498, 7509, 7521, 7532, 7544, 7555, 7566, 7578, 7589, 7601, 7612, 7623, 7635, 7646, 7657, 7668,
		7679, 7691, 7702, 7713, 7724, 7735, 7746, 7757, 7768, 7779, 7790, 7801, 7812, 7823, 7834, 7845,
		7856, 7866, 7877, 7888, 7899, 7910, 7920, 7931, 7942, 7952, 7963, 7974, 7984, 7995, 8005, 8016,
		8026, 8037, 8047, 8058, 8068, 8079, 8089, 8100, 8110, 8120, 8131, 8141, 8151, 8161, 8172, 8182,
		8192
	};
	const uint16_t _360_DEG_16B = 0xFFFF;
	const int16_t _180_DEG_16B = 32767;
	const int16_t _90_DEG_16B = 16383;
	int16_t divide;
#define Q15(X) \
   ((X < 0.0) ? (int16_t)(32768*(X) - 0.5) : (int16_t)(32767*(X) + 0.5))




	int MATHS_atan_16(int y, int x)
	{
		int32_t absX = x;
		int32_t absY = y;
		uint16_t angle;


		if (x < 0)
		{
			absX = -x;
		}
		if (y < 0)
		{
			absY = -y;
		}

		if (absY > absX)
		{
			divide = (absX << ARG_RESOLUTION_HI) / absY;

			//int16_t null;
			//divide = __builtin_divmodud((absX << ARG_RESOLUTION_HI), absY, &null);


			/* upper octant, 45.01° ~ 90° */
			angle = _90_DEG_16B - ATAN_TABLE[divide]; // 90 degees
		}
		else
		{
			/* lower octant, 0° ~ 45° */
			divide = (absY << ARG_RESOLUTION_HI) / absX;
			//	int16_t null;
			//	divide = __builtin_divmodud((absY << ARG_RESOLUTION_HI), absX, &null);
			angle = ATAN_TABLE[divide];
		}

		if (y < 0)
		{
			/* 180.01° ~ 359.99° */
			if (x < 0)
			{
				/* 180.01° ~ 269.99°, angle = angle - 180° */
				angle = _180_DEG_16B + angle;
			}
			else
			{
				/* 270° ~ 359.99°, angle = 360° - angle */
				angle = _360_DEG_16B - angle;
			}
		}
		else if (x < 0)
		{
			/* 90.01° ~ 180°, angle = 180° - angle */
			angle = _180_DEG_16B - angle;
		}
		else
		{
			/* 0° ~ 90° */
			/* Do nothing. */
		}
		return angle;   /* 0-65535 -> 0°-359.99° */
	}


	// Goes nowhere, but good for unit testing ATAN2
	int16_t atan2_test_angle;

	void atan2_unit_test(void)
	{
		atan2_test_angle = MATHS_atan_16(Q15(0.389418342), Q15(0.921060994)); //4172

		atan2_test_angle = MATHS_atan_16(Q15(0.863209367), Q15(-0.504846105)); //21903

		atan2_test_angle = MATHS_atan_16(Q15(-0.611857891), Q15(-0.790967712)); //-25900

		atan2_test_angle = MATHS_atan_16(Q15(-0.373876665), Q15(0.927478431)); //-3996
	}


	void packRGB332_extend16(uint8_t red, uint8_t green, uint8_t blue, uint16_t* colourcode)
	{
		//R4,R3,B5,B4,B3,G4,G3,G2	B7	B6	G7	G6	G5	R7	R6	R5

		// Lower 8 bits
		*colourcode = red >> 5;
		*colourcode |= (green & 0xE0) >> 2;
		*colourcode |= (blue & 0xc0);
		// Upper 8 bits
		*colourcode |= (red & 0x18) << 5;
		*colourcode |= (green & 0x18) << 7;
		*colourcode |= (blue & 0x38) << 9;

	}

	void packRGB332(uint8_t red, uint8_t green, uint8_t blue, uint8_t* colourcode)
	{
		*colourcode = red >> 5 & 0x7;
		*colourcode |= (green >> 5 & 0x7) << 3;
		*colourcode |= (blue >> 6 & 0x3) << 6;
	}

#if 0
	__inline int16_t mul16_i(int16_t x, int16_t y)
	{
		int32_t x32 = x;
		x32 *= y;
		x32 >>= 15;

		return (int16_t)x32;
	}

	__inline int16_t mul16_iu(int16_t x, uint16_t y)
	{
		int32_t x32 = x;
		x32 *= y;
		x32 >>= 16;

		return (int16_t)x32;
	}

	__inline uint16_t mul16_uu(uint16_t x, uint16_t y)
	{
		uint32_t x32 = x;
		x32 *= y;
		x32 >>= 16;

		return (uint16_t)x32;
	}


	void packRGB555(uint8_t red, uint8_t green, uint8_t blue, uint16_t* colourcode)
	{
		*colourcode = red >> 3 & 0x1f;
		*colourcode |= (((uint16_t)green) >> 3 & 0x1F) << 5;
		*colourcode |= (((uint16_t)blue) >> 3 & 0x1F) << 10;
	}
	void unpackRGB555(uint16_t colourcode, uint8_t* red, uint8_t* green, uint8_t* blue)
	{
		*red = (colourcode & 0x1f) << 3;
		*green = ((colourcode >> 5) & 0x1f) << 3;
		*blue = ((colourcode >> 10) & 0x1f) << 3;
	}


	void unpackRGB332(uint16_t colourcode, uint8_t* red, uint8_t* green, uint8_t* blue)
	{
		*red = (colourcode & 0x7) << 5;
		*green = ((colourcode >> 3) & 0x7) << 5;
		*blue = ((colourcode >> 6) & 0x3) << 6;

	}
#endif

	/**********************************************
	 unpack bits
	 not part of PICO - used by PC simulator/emulator
	 Needs to move into a class or something C++'y when I can work out where
	 LSB in upper 8, MSB in 332 in lower 8
	 R4, R3, B5, B4, B3, G4, G3, G2, B7, B6, G7, G6, G5, R7, R6, R5

	 */

	void unpackRGB555_dualmodes(uint16_t colourcode, uint8_t* red, uint8_t* green, uint8_t* blue)
	{
		// MSB 
		*red = (colourcode & 0x7) << 5;
		*green = (colourcode << 0x2) & 0xE0;
		*blue = (colourcode & 0xc0);

		//LSB

		*red |= (colourcode >> 5) & 0x18;
		*green |= ((colourcode >> 7) & 0x18);
		*blue |= ((colourcode >> 9) & 0x38);
	}





}
