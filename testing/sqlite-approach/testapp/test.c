#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

static void updateSensor(sqlite3* db, int id, int value);

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf ("Usage: %s db_location\n", argv[0]);
		return 1;
	}

	sqlite3* db;
	int ret;

	if (ret = sqlite3_open(argv[1], &db) != SQLITE_OK) {
		printf("Error opening database: %s\n",
			sqlite3_errstr(ret));
		return 1;
	}

	sqlite3_stmt *statement;
	char *query = "insert into sensors values(3,4)";
	if (ret = sqlite3_prepare(db, query, (int)strlen(query) + 1, &statement, NULL) != SQLITE_OK) {
		printf("Error in query: %s\n",
			sqlite3_errstr(ret));
	}

	sqlite3_step(statement);
	sqlite3_finalize(statement);

	updateSensor(db, 44, 2332);
	sqlite3_close(db);
	return 0;
}

static void updateSensor(sqlite3* db, int id, int value) {
	char query[BUFSIZ];
	snprintf(query, BUFSIZ, "update sensors set value=%d where id=%d", value, id);

	//Needs error checking
	int ret;
	if (ret = sqlite3_exec(db, query, NULL, NULL, NULL) != SQLITE_OK) {
		printf("Error: %s\n", sqlite3_errstr(ret));
	}
	if (!sqlite3_changes(db)) {
		printf("Record doesn't exist; creating!\n");
		snprintf(query, BUFSIZ, "insert into sensors values(%d, %d)", id, value);
		sqlite3_exec(db, query, NULL, NULL, NULL);
	}
}
