#ifndef _domparser_h_
#define _domparser_h_

enum { EATTR_VOID=1, EATTR_SINGLE_QUOTES=2, EATTR_DOUBLE_QUOTES=3 };
enum { ETAG_SELF=1, ETAG_END };

typedef struct _domattr_t
{
	int eattr;
	char* name;
	char* value; // 0-alone(<input type="checked" checked>)
	struct _domattr_t* prev;
	struct _domattr_t* next;
} domattr_t;

typedef struct _domnode_t
{
	struct _domnode_t* parent;
	struct _domnode_t* child;
	struct _domnode_t* prev;
	struct _domnode_t* next;
	struct _domnode_t* end; // 0-void(<br>), 1-self-end(<meta/>), 2-end-tag(</head>), other-end-tag-pointer
	char* padding;
	char* name;		// 0-text node, other-tag node
	domattr_t* attr;
} domnode_t;

typedef struct
{
	char* encoding; // document encoding(<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />)
	char* error;
	char* errpos;
	const char* perr;
	const char* p;

	domnode_t* root;
} domdoc_t;

// parse html document
domdoc_t* domparser_parse(const char* xhtml);

// parse html tag
// parse <body onload="OnLoad()">
// parse <meta keyword="keyword" />
// parse <br>
const char* domparser_parsenode(domdoc_t* doc, domnode_t* node, const char* p);

// set parse error
void domparser_seterror(domdoc_t* doc, const char* perr, const char* errmsg);

// set encoding(charset)
void domparser_setencoding(domdoc_t* doc, const char* encoding);

// only for html tag.
// can't for <?php .... ?>
int domparser_append(domnode_t* parent, domnode_t* node);

#endif /* !_domparser_h_ */
