#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ctype.h>
#include <ndb.h>
#include "ndbhf.h"

/*
 *  Parse a data base entry.  Entries may span multiple
 *  lines.  An entry starts on a left margin.  All subsequent
 *  lines must be indented by white space.  An entry consists
 *  of tuples of the forms:
 *	attribute-name
 *	attribute-name=value
 *	attribute-name="value with white space"
 *
 *  The parsing returns a 2-dimensional structure.  The first
 *  dimension joins all tuples. All tuples on the same line
 *  form a ring along the second dimension.
 */

/*
 *  parse the next entry in the file
 */
Ndbtuple*
ndbparse(Ndb *db)
{
	char *line;
	Ndbtuple *t;
	Ndbtuple *first, *last;
	int len;

	first = last = nil;
	for(;;){
		if((line = Brdline(&db->b, '\n')) == nil)
			break;
		len = Blinelen(&db->b);
		if(line[len-1] != '\n')
			break;
		if(first && !ISWHITE(*line) && *line != '#'){
			Bseek(&db->b, -len, 1);
			break;
		}
		t = _ndbparseline(line);
		if(t == nil)
			continue;
		setmalloctag(t, getcallerpc(&db));
		if(first != nil)
			last->entry = t;
		else
			first = t;
		last = t;
		while(last->entry != nil)
			last = last->entry;
	}
	ndbsetmalloctag(first, getcallerpc(&db));
	return first;
}
