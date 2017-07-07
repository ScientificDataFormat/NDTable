#include "catch.hpp"

#define _USE_MATH_DEFINES  // for M_PI
#include <math.h>

#include "NDTable.h"


//#ifdef _MSC_VER
//static const unsigned long __nan[2] = { 0xffffffff, 0x7fffffff };
//#define NAN (*(const float *) __nan)
//#define INFINITY (DBL_MAX + DBL_MAX)
//#endif

#define N_NON_FINITE 3
static const double non_finite[N_NON_FINITE] = { NAN, INFINITY, -INFINITY }; 

TEST_CASE("Interplation") {

	SECTION("Find index") {
		double value = 1.2;
		double values[4] = { 0, 1, 2, 3 };
		int num_values = 4;
		int index = -1;
		double t = 0;
		
		NDTable_find_index(value, num_values, values, &index, &t, NDTABLE_EXTRAP_HOLD);
		
		REQUIRE(index == 1);
	}

	SECTION("Interpolate nearest") {
		double x[2] = { 0, 1 };
		double y[2] = { 2, 3 };
		
		NDTable_t ds;
		ds.data = y;
		ds.scales[0] = x;
		ds.dims[0] = 2;
		ds.numel = 2;
		ds.ndims = 1;
		ds.offs[0] = 1;
		
		int subs[1] = { 0 };
		int nsubs[1];
		double t[1];
		
		double value;
		double derivatives[1];
				
		// interpolate left boundary
		t[0] = 0.0;
		REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_NEAREST, NDTABLE_EXTRAP_NONE, &value, derivatives));
		REQUIRE(2.0 == value);
		REQUIRE(0.0 == derivatives[0]);
		
		// interpolate before step
		t[0] = 0.49;
		REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_NEAREST, NDTABLE_EXTRAP_NONE, &value, derivatives));
		REQUIRE(2.0 == value);
		REQUIRE(0.0 == derivatives[0]);
		
		// interpolate after step
		t[0] = 0.51;
		REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_NEAREST, NDTABLE_EXTRAP_NONE, &value, derivatives));
		REQUIRE(3.0 == value);
		REQUIRE(0.0 == derivatives[0]);
		
		// interpolate right boundary
		t[0] = 1.0;
		REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_NEAREST, NDTABLE_EXTRAP_NONE, &value, derivatives));
		REQUIRE(3.0 == value);
		REQUIRE(0.0 == derivatives[0]);
		
		// non-finite values
		for(int i = 0; i < N_NON_FINITE; i++) {
			// left
			y[0] = non_finite[i]; 
			y[1] = 3;
			t[0] = 0.0;
			REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_NEAREST, NDTABLE_EXTRAP_LINEAR, &value, derivatives));
			REQUIRE(1 == _isnan(value));
			REQUIRE(1 == _isnan(derivatives[0]));
		
			// right boundary
			y[0] = 3;
			y[1] = non_finite[i]; 
			t[0] = 1.0;
			REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_NEAREST, NDTABLE_EXTRAP_LINEAR, &value, derivatives));
			REQUIRE(1 == _isnan(value));
			REQUIRE(1 == _isnan(derivatives[0]));
		}
	}

	SECTION("Interplate linear") {
		double x[2] = { 0, 1 };
		double y[2] = { 2, 3 };
		
		NDTable_t ds;
		ds.data = y;
		ds.scales[0] = x;
		ds.dims[0] = 2;
		ds.numel = 2;
		ds.ndims = 1;
		ds.offs[0] = 1;
		
		int subs[1] = { 0 };
		int nsubs[1];
		double t[1] = { 0.3 };
		
		double value;
		double derivatives[1];
				
		REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_LINEAR, NDTABLE_EXTRAP_NONE, &value, derivatives));
		REQUIRE(2.3 == value);
		REQUIRE(1.0 == derivatives[0]);
		
		// non-finite values
		for(int i = 0; i < N_NON_FINITE; i++) {
			// left value non-finite
			y[0] = non_finite[i];
			y[1] = 2;
			REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_LINEAR, NDTABLE_EXTRAP_NONE, &value, derivatives));
			REQUIRE(1 == _isnan(value));
			REQUIRE(1 == _isnan(derivatives[0]));
		
			// right value non-finite
			y[0] = non_finite[i];
			y[1] = 3;
			REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_LINEAR, NDTABLE_EXTRAP_NONE, &value, derivatives));
			REQUIRE(1 == _isnan(value));
			REQUIRE(1 == _isnan(derivatives[0]));
		}
	}

	SECTION("Interpolate akima") {
		#define N 8
		
		int i, j, k;
		
		double x[N];
		double y[N];
		
		for (i = 0; i < N; i++) {
			x[i] = i * ((double)N) / ((double) N-1);
			y[i] = sin(x[i] * M_PI);
		}
			
		NDTable_t ds;
		ds.data = y;
		ds.scales[0] = x;
		ds.dims[0] = N;
		ds.numel = N;
		ds.ndims = 1;
		ds.offs[0] = 1;
		
		int subs[1] = { 1 };
		int nsubs[1];
		double t[1] = { 0.99 };
		
		double value;
		double derivatives[1];
		
		REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_AKIMA, NDTABLE_EXTRAP_NONE, &value, derivatives));
		
		// non-finite values
		subs[0] = 2;
		t[0] = 0.5;
		for(i = 0; i < N_NON_FINITE; i++) {
			for(j = 0; j < N; j++) {
				for (k = 0; k < N; k++) {
					y[i] = sin(x[i] * M_PI);
				}
				y[j] = non_finite[i];
				REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_AKIMA, NDTABLE_EXTRAP_NONE, &value, derivatives));
				if (!_isnan(value)) {
					i = i;
				}
				REQUIRE(1 == _isnan(value));
				REQUIRE(1 == _isnan(derivatives[0]));
			}
		}

	}

	SECTION("Extrapolate hold") {
		double x[2] = { 0, 1 };
		double y[2] = { 2, 3 };
		
		NDTable_t ds;
		ds.data = y;
		ds.scales[0] = x;
		ds.dims[0] = 2;
		ds.numel = 2;
		ds.ndims = 1;
		ds.offs[0] = 1;
		
		int subs[1] = { 0 };
		int nsubs[1];
		double t[1];
		
		double value;
		double derivatives[1];
		
		int i;
		
		// extrapolate left
		t[0] = -0.1;
		REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_NEAREST, NDTABLE_EXTRAP_HOLD, &value, derivatives));
		REQUIRE(2.0 == value);
		REQUIRE(0.0 == derivatives[0]);
		
		// extrapolate right
		t[0] = 1.1;
		REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_NEAREST, NDTABLE_EXTRAP_HOLD, &value, derivatives));
		REQUIRE(3.0 == value);
		REQUIRE(0.0 == derivatives[0]);
		
		// non-finite values
		for(i = 0; i < N_NON_FINITE; i++) {
			y[0] = y[1] = non_finite[i];
		
			// left
			t[0] = -0.1;
			REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_NEAREST, NDTABLE_EXTRAP_HOLD, &value, derivatives));
			REQUIRE(1 == _isnan(value));
			REQUIRE(1 == _isnan(derivatives[0]));
		
			// right
			t[0] = 1.1;
			REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_NEAREST, NDTABLE_EXTRAP_HOLD, &value, derivatives));
			REQUIRE(1 == _isnan(value));
			REQUIRE(1 == _isnan(derivatives[0]));
		}
	}

	SECTION("Extrapolate linear") {
		double x[2] = { 0, 1 };
		double y[2] = { 2, 3 };
		
		NDTable_t ds;
		ds.data = y;
		ds.scales[0] = x;
		ds.dims[0] = 2;
		ds.numel = 2;
		ds.ndims = 1;
		ds.offs[0] = 1;
		
		int subs[1] = { 0 };
		int nsubs[1];
		double t[1];
		
		double value;
		double derivatives[1];
		
		int i, j;
		
		// extrapolate left
		t[0] = -1.0;
		REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_NEAREST, NDTABLE_EXTRAP_LINEAR, &value, derivatives));
		REQUIRE(1.0 == value);
		REQUIRE(1.0 == derivatives[0]);
		
		// extrapolate right
		t[0] = 2.0;
		REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_NEAREST, NDTABLE_EXTRAP_LINEAR, &value, derivatives));
		REQUIRE(4.0 == value);
		REQUIRE(1.0 == derivatives[0]);
		
		// non-finite values
		for(i = 0; i < N_NON_FINITE; i++) {
			for(j = 0; j < 2; j++) {
				y[0] = 2; y[1] = 3;
				y[j] = non_finite[i];
				REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_NEAREST, NDTABLE_EXTRAP_LINEAR, &value, derivatives));
				REQUIRE(1 == _isnan(value));
				REQUIRE(1 == _isnan(derivatives[0]));
			}
		}
	}

	SECTION("Extrapolate none") {
		double x[2] = { 0, 1 };
		double y[2] = { 2, 3 };
		
		NDTable_t ds;
		ds.data = y;
		ds.scales[0] = x;
		ds.dims[0] = 2;
		ds.numel = 2;
		ds.ndims = 1;
		ds.offs[0] = 1;
		
		int subs[1] = { 0 };
		int nsubs[1];
		double t[1];
		
		double value;
		double derivatives[MAX_NDIMS];
			
		// extrapolate left
		t[0] = -1;
		REQUIRE(-1 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_NEAREST, NDTABLE_EXTRAP_NONE, &value, derivatives));
		
		// extrapolate right
		t[0] = 2.0;
		REQUIRE(-1 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_NEAREST, NDTABLE_EXTRAP_NONE, &value, derivatives));
	}

	SECTION("Dummy dimension 1-d") {
		// this tests checks the direct return of the sample value
		// without interpolation for dimensions with extent < 2
		
		// create a 1-d dataset with length 1 
		double x[1] = { 0 };
		double y[1] = { 1.1 };
		
		NDTable_t ds;
		ds.data = y;
		ds.scales[0] = x;
		ds.dims[0] = 1;
		ds.numel = 1;
		ds.ndims = 1;
		ds.offs[0] = 1;
		
		int subs[1] = { 0 };
		int nsubs[1] = { 0 }; // not used
		double t[1] = { 0.0 };
		
		double value;
		double derivative;
		
		REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_NEAREST, NDTABLE_EXTRAP_NONE, &value, &derivative));
		REQUIRE(y[0] == value);
		REQUIRE(0.0 == derivative);
	}

	SECTION("Dummy dimension 2-d") {
		// this tests checks the direct return of the sample value
		// without interpolation for dimensions with extent < 2
		
		// create a 2-d dataset with size (2,1) 
		double x[2] = { 0, 1 };
		double y[1] = { 1 };
		double z[2] = { 1, 2 };
		
		NDTable_t ds;
		ds.data = z;
		ds.scales[0] = x;
		ds.scales[1] = y;
		ds.dims[0] = 2;
		ds.dims[1] = 1;
		ds.numel = 2;
		ds.ndims = 2;
		ds.offs[0] = 1;
		ds.offs[1] = 2;
		
		int subs[2] = { 0, 0 };
		int nsubs[2];
		double t[2];
		
		double value;
		double der_values[2];
		
		t[0] = -0.1; 
		t[1] = -0.1;
		REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_LINEAR, NDTABLE_EXTRAP_HOLD, &value, der_values));
		REQUIRE(1.0 == value);
		REQUIRE(0.0 == der_values[0]);
		REQUIRE(0.0 == der_values[1]);
		
		t[0] = 0.5; 
		t[1] = 0.5;
		REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_LINEAR, NDTABLE_EXTRAP_HOLD, &value, der_values));
		REQUIRE(1.5 == value);
		REQUIRE(1.0 == der_values[0]);
		REQUIRE(0.0 == der_values[1]);
		
		t[0] = 1.1; 
		t[1] = 1.1;
		REQUIRE(0 == NDTable_evaluate_internal(&ds, t, subs, nsubs, 0, NDTABLE_INTERP_LINEAR, NDTABLE_EXTRAP_HOLD, &value, der_values));
		REQUIRE(2.0 == value);
		REQUIRE(0.0 == der_values[0]);
		REQUIRE(0.0 == der_values[1]);
	}

}
