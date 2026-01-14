# dbstorage_sqlstore API Reference

## Index

- Class `DBStorageSQLStoreError`
- Class `SQLStore`

---

## `DBStorageSQLStoreError` class

```python
class DBStorageSQLStoreError()
```

Exception raised for SQLite database operations errors.

This exception is raised when database operations fail, such as connection
errors, SQL syntax errors, constraint violations, or table access issues.


---

## `SQLStore` class

```python
class SQLStore(database_name: str)
```

SQLStore client for storing and retrieving data in a SQLite database.

This class provides methods to create tables, insert, read, update, and delete records,
and execute raw SQL commands. It uses SQLite as the underlying database engine and
supports named access to columns using sqlite3.Row as the row factory.
It is designed to be thread-safe and can be used in multi-threaded applications.

### Parameters

- **database_name** (*str*) (optional): Name of the SQLite database file.
Defaults to "arduino.db".

### Methods

#### `start()`

Open the SQLite database connection.

This method establishes the database connection and should be called before
performing any database operations. The connection is thread-safe and enables
named column access using sqlite3.Row factory.

##### Raises

- **DBStorageSQLStoreError**: If there is an error starting the SQLite connection.

#### `stop()`

Close the SQLite database connection.

##### Raises

- **DBStorageSQLStoreError**: If there is an error stopping the SQLite connection.

#### `create_table(table: str, columns: dict[str, str])`

Create a table in the SQLite database if it does not already exist.

##### Parameters

- **table** (*str*): Name of the table to create.
- **columns** (*dict[str, str]*): Dictionary mapping column names to SQL types.
Common types: "INTEGER", "REAL", "TEXT", "BLOB", "INTEGER PRIMARY KEY".

##### Raises

- **DBStorageSQLStoreError**: If there is an error creating the table.

#### `drop_table(table: str)`

Remove a table and all its data from the database. This permanently deletes the table and all its data.

##### Parameters

- **table** (*str*): Name of the table to drop.

##### Raises

- **DBStorageSQLStoreError**: If there is an error dropping the table.

#### `store(table: str, data: dict[str, Any], create_table: bool)`

Store data in the specified table with automatic table creation. By default, it creates the table if it doesn't exist.

##### Parameters

- **table** (*str*): Name of the table to store the record in.
- **data** (*dict[str, Any]*): Dictionary of column names and their values.
Supported types: int (INTEGER), float (REAL), str (TEXT), bytes (BLOB).
- **create_table** (*bool*) (optional): If True, create the table if it doesn't exist
using automatic type inference. Defaults to True.

##### Raises

- **DBStorageSQLStoreError**: If there is an error inserting data or creating the table.

#### `read(table: str, columns: Optional[list], condition: Optional[str], order_by: Optional[str], limit: Optional[int])`

Get data from the specified table with flexible filtering options. If the table does not exist, it returns an empty list.

##### Parameters

- **table** (*str*): Name of the table to read from.
- **columns** (*Optional[list]*) (optional): List of column names to select.
If None, selects all columns. Defaults to None.
- **condition** (*Optional[str]*) (optional): WHERE clause for filtering results
(e.g., "age > 18"). Defaults to None.
- **order_by** (*Optional[str]*) (optional): ORDER BY clause for sorting results
(e.g., "name ASC"). Defaults to None.
- **limit** (*Optional[int]*) (optional): Maximum number of rows to return.
Use -1 for no limit. Defaults to -1.

##### Returns

- (*list[dict[str, Any]]*): List of dictionaries representing the rows, where each
dictionary maps column names to their values. Empty list if table doesn't exist.

##### Raises

- **DBStorageSQLStoreError**: If there is an error reading data from the table.

#### `update(table: str, data: dict[str, Any], condition: Optional[str])`

Update data or records in the specified table.

##### Parameters

- **table** (*str*): Name of the table to update.
- **data** (*dict[str, Any]*): Dictionary of column names and their new values.
- **condition** (*Optional[str]*) (optional): WHERE clause for filtering which records
to update (e.g., "id = 1"). If empty, updates all records. Defaults to "".

##### Raises

- **DBStorageSQLStoreError**: If there is error updating data in the table.

#### `delete(table: str, condition: Optional[str])`

Delete data from the specified table. If no condition is provided, this will delete ALL records from the table.

##### Parameters

- **table** (*str*): Name of the table to delete from.
- **condition** (*Optional[str]*) (optional): WHERE clause for filtering which records
to delete (e.g., "age < 18"). If empty, deletes all records. Defaults to "".

##### Raises

- **DBStorageSQLStoreError**: If there is an error deleting data from the table.

#### `execute_sql(sql: str, args: Optional[tuple])`

Execute a raw SQL command.

##### Parameters

- **sql** (*str*): The SQL command to execute.
- **args** (*Optional[tuple]*): Optional parameters for the SQL command.

##### Returns

-: list[dict[str, Any]] | None: A list of dictionaries representing the rows returned by the SQL command,
or None if the command does not return any rows.

##### Raises

- **DBStorageSQLStoreError**: If there is an error executing the SQL command.

#### `create_or_replace_table(table: str, columns: dict[str, str], force_drop_table: bool)`

Create or update a table in the SQLite database to match the provided schema.

All schema changes (adding/removing/changing columns) are performed within a single transaction.
If any error occurs during the operation due to SQLite limitations or constraints, the transaction is rolled back
, and the table remains unchanged. If force_drop_table is True, after rollback, the table is dropped and recreated.

If the table exists, it will add missing columns and remove extra columns unless they are not-simple columns.
(e.g., primary key, unique, indexed, or used in constraints/triggers/views).

If a column's type has changed or if a column is not simple, it will raise an error unless
force_drop_table is True, in which case the table is dropped and recreated with the new schema, losing all
existing data in the table.

##### Parameters

- **table** (*str*): Name of the table to create or update.
- **columns** (*dict[str, str]*): Dictionary of column names and their SQL types.
- **force_drop_table** (*bool*): If True, always drop and recreate the table if schema change fails.

##### Raises

- **DBStorageSQLStoreError**: If there is an error creating or updating the table. All changes are rolled back.

