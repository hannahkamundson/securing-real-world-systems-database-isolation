package io.digit;

import io.digit.commands.Command;
import lombok.extern.slf4j.Slf4j;

@Slf4j
class DatabaseRPCImpl implements DatabaseRPC {
    @Override
    public boolean ready() {
        log.info("The database is ready");
        return true;
    }

//    @Override
//    public void run(Command command) {
////        log.info("The database is running");
//
//        // TODO
////        // Execute the command
////        try(Connection connection = DriverManager.getConnection("jdbc:sqlite:" + DATABASE_NAME)) {
////            Statement statement = connection.createStatement();
////            command.execute(statement);
////        } catch (SQLException e) {
////            throw new RuntimeException(e);
////        }
//    }
}
