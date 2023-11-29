# securing-real-world-systems-database-isolation

## Building the project
You will need to have the following installed:
- [Java >= 11](https://openjdk.org/install/)
- [Maven >= 3.9.5](https://maven.apache.org/install.html)

To build the project, go into the base directory securing-real-world-systems-database-isolation and run the following command
```shell
mvn clean install
```

This will build your application jar in [app/target/app-1.0-SNAPSHOT.jar](app/target/app-1.0-SNAPSHOT.jar)

## Running the project
Ensure you have built the project above. When you run the program, you need to decide on three things:
1. iterations: how many times do you want to run the SQL command in the timing?
2. command: what SQL command do you want to test?
   1. Look in [CommandType](db/src/main/java/io/digit/commands/CommandType.java) to see the options
3. sandbox: do you want to sandbox the JDBC driver? or do you want to run it normally?
   1. Normally: [NONE](app/src/main/java/io/digit/sandbox/SandboxType.java)
   2. Process isolation sandbox: [PROCESS](app/src/main/java/io/digit/sandbox/SandboxType.java)

Once you've decided on this, you can run the following command from the base directory
```shell
java -jar app/target/app-1.0-SNAPSHOT.jar --iterations <iterations> --command <command> --sandbox <sandbox>
```