-- CREATION OF THE SIGFOX TABLES WITH SOME DATA


-- Delete the tables if they exists
DROP TABLE IF EXISTS `raws`;
DROP TABLE IF EXISTS `events`;
DROP TABLE IF EXISTS `devices`;


-- --
-- -- Create 'raws' table
-- --
CREATE TABLE IF NOT EXISTS `raws` (
  `idraws` INTEGER PRIMARY KEY,
  `time` INTEGER NOT NULL,
  `device` TEXT NOT NULL,
  `snr` REAL NOT NULL,
  `station` TEXT NOT NULL,
  `ack` TEXT,
  `data` TEXT NOT NULL,
  `duplicate` TEXT NOT NULL,
  `avgSignal` REAL NOT NULL,
  `rssi` REAL NOT NULL,
  `longPolling` TEXT,
  `seqNumber` INTEGER NOT NULL
);


-- --
-- -- Create 'events' table
-- --
CREATE TABLE IF NOT EXISTS `events` (
  `idevents` INTEGER PRIMARY KEY,
  `idmodem` TEXT NOT NULL,
  `time` INTEGER NOT NULL,
  `event_type` INTEGER NOT NULL,
  `temperature` REAL NOT NULL,
  `longitude` INTEGER NOT NULL,
  `latitude` INTEGER NOT NULL,
  `altitude` INTEGER NOT NULL,
  `sign_longitude` INTEGER NOT NULL,
  `sign_latitude` INTEGER NOT NULL,
  `sign_altitude` INTEGER NOT NULL,
  `satellite` INTEGER NOT NULL,
  `precision` INTEGER NOT NULL,
  `battery` INTEGER NOT NULL
);


-- --
-- -- Create 'devices' table
-- --
CREATE TABLE IF NOT EXISTS `devices` (
  `iddevices` INTEGER PRIMARY KEY,
  `idmodem` TEXT NOT NULL,
  `attribution` INTEGER,
  `timestamp_attribution` INTEGER
);