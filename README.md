# securing-real-world-systems-database-isolation


### run performance
java -jar SPECjvm2008_1_01_setup.jar 

#### check tables in sqlite
SELECT name FROM sqlite_schema
WHERE type IN ('table','view')
AND name NOT LIKE 'sqlite_%'
ORDER BY 1;