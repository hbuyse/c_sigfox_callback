-- CREATION OF THE SIGFOX TABLES WITH SOME DATA

--
-- Create 'raws' table
--
CREATE TABLE IF NOT EXISTS `raws` (
  `idraws` INTEGER PRIMARY KEY AUTOINCREMENT,
  `time` INTEGER NOT NULL,
  `idmodem` TEXT NOT NULL,
  `snr` REAL NOT NULL,
  `station` TEXT NOT NULL,
  `ack` integer,
  `data` TEXT NOT NULL,
  `duplicate` INTEGER NOT NULL,
  `avgSignal` REAL NOT NULL,
  `rssi` REAL NOT NULL,
  `lat` INTEGER NOT NULL,
  `lon` INTEGER NOT NULL,
  `seqNumber` INTEGER NOT NULL
);


--
-- Create 'devices' table
--
CREATE TABLE IF NOT EXISTS `devices` (
  `iddevices` INTEGER PRIMARY KEY AUTOINCREMENT,
  `idmodem` TEXT NOT NULL UNIQUE,
  `attribution` INTEGER,
  `timestamp_attribution` INTEGER
);