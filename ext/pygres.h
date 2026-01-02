#ifndef _PYGRES_H
#define _PYGRES_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <libpq-fe.h>
#include <libpq/libpq-fs.h>

/* The type definitions from <server/catalog/pg_type.h> */
#include "pgtypes.h"

extern PyObject *Error, *Warning, *InterfaceError, *DatabaseError,
    *InternalError, *OperationalError, *ProgrammingError, *IntegrityError,
    *DataError, *NotSupportedError, *InvalidResultError, *NoResultError,
    *MultipleResultsError, *Connection, *Query, *LargeObject;

/* Default values */
#define PG_ARRAYSIZE 1

/* Flags for object validity checks */
#define CHECK_OPEN 1
#define CHECK_CLOSE 2
#define CHECK_CNX 4
#define CHECK_RESULT 8
#define CHECK_DQL 16

/* Query result types */
#define RESULT_EMPTY 1
#define RESULT_DML 2
#define RESULT_DDL 3
#define RESULT_DQL 4

#define MAX_ARRAY_DEPTH 16    /* maximum allowed depth of an array */

/* Simple types */
#define PYGRES_INT 1
#define PYGRES_LONG 2
#define PYGRES_FLOAT 3
#define PYGRES_DECIMAL 4
#define PYGRES_MONEY 5
#define PYGRES_BOOL 6
/* Text based types */
#define PYGRES_TEXT 8
#define PYGRES_BYTEA 9
#define PYGRES_JSON 10
#define PYGRES_OTHER 11
/* Array types */
#define PYGRES_ARRAY 16

/* Forward declarations for types */
extern PyTypeObject connType, sourceType, queryType, noticeType, largeType;

/* Object declarations */

typedef struct {
    PyObject_HEAD int valid;   /* validity flag */
    PGconn *cnx;               /* Postgres connection handle */
    const char *date_format;   /* date format derived from datestyle */
    PyObject *cast_hook;       /* external typecast method */
    PyObject *notice_receiver; /* current notice receiver */
} connObject;
#define is_connObject(v) (PyType(v) == &connType)

typedef struct {
    PyObject_HEAD int valid; /* validity flag */
    connObject *pgcnx;       /* parent connection object */
    PGresult *result;        /* result content */
    int encoding;            /* client encoding */
    int result_type;         /* result type (DDL/DML/DQL) */
    long arraysize;          /* array size for fetch method */
    int current_row;         /* currently selected row */
    int max_row;             /* number of rows in the result */
    int num_fields;          /* number of fields in each row */
} sourceObject;
#define is_sourceObject(v) (PyType(v) == &sourceType)

typedef struct {
    PyObject_HEAD connObject *pgcnx; /* parent connection object */
    PGresult const *res;             /* an error or warning */
} noticeObject;
#define is_noticeObject(v) (PyType(v) == &noticeType)

typedef struct {
    PyObject_HEAD connObject *pgcnx; /* parent connection object */
    PGresult *result;                /* result content */
    int async;                       /* flag for asynchronous queries */
    int encoding;                    /* client encoding */
    int current_row;                 /* currently selected row */
    int max_row;                     /* number of rows in the result */
    int num_fields;                  /* number of fields in each row */
    int *col_types;                  /* PyGreSQL column types */
} queryObject;
#define is_queryObject(v) (PyType(v) == &queryType)

typedef struct {
    PyObject_HEAD connObject *pgcnx; /* parent connection object */
    Oid lo_oid;                      /* large object oid */
    int lo_fd;                       /* large object fd */
} largeObject;
#define is_largeObject(v) (PyType(v) == &largeType)

/*
   A buffer for character data with routines to handle resizing.
   This is inspired by libpq's PQExpBufferData.
   The buffer can be extended with the extend_char_buffer_s/x() functions.
*/
struct CharBuffer {
    char *data;      /* actual string data */
    size_t size;     /* current size of data */
    size_t max_size; /* allocated size */
    int error;       /* error flag (invalid data) */
};


/* MODULE GLOBAL VARIABLES from pgmodule.c */
extern PyObject *decimal, *dictiter, *namediter, *namednext, *scalariter, *jsondecode;

extern const char *date_format;
extern int pg_encoding_utf8;
extern int pg_encoding_latin1;
extern int pg_encoding_ascii;

extern char decimal_point;
extern int bool_as_text;
extern int array_as_text;
extern int bytea_escaped;

// pginternal.c
extern PyObject *get_decoded_string(const char *str, Py_ssize_t size, int encoding);
extern PyObject *get_encoded_string(PyObject *unicode_obj, int encoding);
extern int get_type(Oid pgtype);
extern int *get_col_types(PGresult *result, int nfields);
extern PyObject *cast_bytea_text(char *s);
extern PyObject *cast_sized_text(char *s, Py_ssize_t size, int encoding, int type);
extern PyObject *cast_other(char *s, Py_ssize_t size, int encoding, Oid pgtype, PyObject *cast_hook);
extern PyObject *cast_sized_simple(char *s, Py_ssize_t size, int type);
extern PyObject *cast_unsized_simple(char *s, int type);
extern PyObject *cast_array(char *s, Py_ssize_t size, int encoding, int type, PyObject *cast, char delim);
extern PyObject *cast_record(char *s, Py_ssize_t size, int encoding, int *type, PyObject *cast, Py_ssize_t len, char delim);
extern PyObject *cast_hstore(char *s, Py_ssize_t size, int encoding);
extern PyObject *get_error_type(const char *sqlstate);
extern void set_error_msg_and_state(PyObject *type, const char *msg, int encoding, const char *sqlstate);
extern void set_error_msg(PyObject *type, const char *msg);
extern void set_error(PyObject *type, const char *msg, PGconn *cnx, PGresult *result);
extern PyObject *get_ssl_attributes(PGconn *cnx);
extern PyObject *format_result(const PGresult *res);
extern const char *date_style_to_format(const char *s);
extern const char *date_format_to_style(const char *s);
extern void notice_receiver(void *arg, const PGresult *res);

extern int init_char_buffer(struct CharBuffer *buf, size_t initial_size);
extern void ext_char_buffer_s(struct CharBuffer *buf, const char *s);
extern void ext_char_buffer_c(struct CharBuffer *buf, char c);

// pgconn.c
extern int _check_cnx_obj(connObject *self);
extern PyObject * _conn_non_query_result(int status, PGresult *result, PGconn *cnx);

#endif /* _PYGRES_H */
