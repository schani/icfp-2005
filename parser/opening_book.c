
#define OPENING_BOOK_DEPTH 16

/* ?? FIXME: 
      robber can escape by going to bank at 55-and-harper ..
 */

char *opening_book[OPENING_BOOK_DEPTH][5] = {
	{ "55-and-kimbark", "54pl-and-woodlawn", "51-and-woodlawn", "53-and-woodlawn", "55-and-s-hyde-park" }, 
	{ "55-and-kenwood", "54-and-woodlawn", "51-and-kimbark", "53-and-s-hyde-park", "55-and-cornell" }, 
	{ "55-and-left-joint", "53-and-woodlawn", "51-and-kenwood", "53-and-cornell", "55-and-lake-park" }, 
	{ "55-and-left-joint", "53-and-kimbark", "51-and-dorchester", "53-and-lake-park", "54pl-and-lake-park" }, 
	{ "55-and-left-joint", "53-and-kenwood", "51-and-dorchester", "52-and-lake-park", "54-and-lake-park" }, 
	{ "55-and-left-joint", "53-and-dorchester", "51-and-dorchester", "52-and-lake-park", "54-and-lake-park" }, 
	{ "55-and-left-joint", "54-and-dorchester", "51-and-dorchester", "52-and-lake-park", "54-and-lake-park" }, 
	{ "55-and-left-joint", "54pl-and-dorchester", "51-and-dorchester", "52-and-lake-park", "54-and-lake-park" }, 


	{ "55-and-dorchester(north)", "54pl-and-dorchester", "51-and-dorchester", "52-and-lake-park", "54-and-lake-park" }, 
	{ "55-and-blackstone(north)", "54pl-and-dorchester", "51-and-dorchester", "52-and-lake-park", "54-and-lake-park" }, 
	{ "rochdale-and-blackstone", "54pl-and-dorchester", "51-and-dorchester", "52-and-lake-park", "54-and-lake-park" }, 
	{ "54pl-and-blackstone", "54pl-and-dorchester", "51-and-dorchester", "52-and-lake-park", "54-and-lake-park" }, 

	{ "54-and-blackstone", "54-and-dorchester", "51-and-dorchester", "52-and-lake-park", "54-and-lake-park" }, 
	{ "53-and-blackstone", "53-and-dorchester", "51-and-dorchester", "52-and-lake-park", "54-and-lake-park" }, 
	{ "52-and-blackstone", "53-and-blackstone", "51-and-dorchester", "52-and-lake-park", "54-and-lake-park" }, 
	{ "52-and-harper(west)", "53-and-harper", "51-and-dorchester", "52-and-lake-park", "54-and-lake-park" }

};

char *opening_move (int turn, int copnr)
{
	if (turn >= OPENING_BOOK_DEPTH) turn = OPENING_BOOK_DEPTH-1;
	return opening_book[turn][copnr]; 
}
