#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "NDTable.h"


TEST_CASE("Core") {
	
	SECTION("Set error message") {
		NDTable_set_error_message("%d plus %.1f equals %s", 1, 1.5, "two point five");
		auto message = NDTable_get_error_message();
		REQUIRE_THAT(message, Catch::Equals("1 plus 1.5 equals two point five"));
	}

	SECTION("Convert index to subscripts with legal index") {
		int subs[3] = { 0 };
		auto ds = NDTable_alloc_table();
			
		ds->ndims = 3;
		ds->offs[0] = 12;
		ds->offs[1] = 4;
		ds->offs[2] = 1;
		
		NDTable_ind2sub(17, ds, subs);
		
		REQUIRE(1 == subs[0]);
		REQUIRE(1 == subs[1]);
		REQUIRE(1 == subs[2]);
	}

	SECTION("Convert subscripts to index with legal subscripts") {
			
		auto ds = NDTable_alloc_table();
		
		ds->ndims = 3;
		ds->dims[0] = 2;
		ds->dims[1] = 3;
		ds->dims[2] = 4;

		int subs[3] = { 1, 1, 1 };
		int index = 0;

		NDTable_sub2ind(subs, ds, &index);
		
		REQUIRE(17 == index);
	}

	SECTION("Allocate and free") {

		// de-allocate with null pointer
		NDTable_free_table(nullptr);
		
		auto ds = NDTable_alloc_table();
		
		// de-allocate with no data
		NDTable_free_table(ds);
		
		// get a new dataset
		ds = NDTable_alloc_table();
		
		// allocate some dummy space
		ds->data = static_cast<double *>(malloc(1));
		
		for(int i = 0; i < MAX_NDIMS; i++) {
			ds->scales[i] = (double *)malloc(1);
		}
		
		// de-allocate with data
		NDTable_free_table(ds);
	}

}
