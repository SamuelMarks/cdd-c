#ifndef C_CDD_DB_LOADER_H
#define C_CDD_DB_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int check_libpq_available(void);
int check_sqlite3_available(void);
int check_mysql_available(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_DB_LOADER_H */
