-- Created by Redgate Data Modeler (https://datamodeler.redgate-platform.com)
-- Last modification date: 2026-04-19 16:08:31.997

-- tables
-- Table: HORSES
CREATE TABLE HORSES (
    id integer NOT NULL CONSTRAINT HORSES_pk PRIMARY KEY,
    nickname varchar(50) NOT NULL,
    age integer,
    experience_years integer,
    owner_id integer NOT NULL,
    CONSTRAINT HORSES_OWNERS FOREIGN KEY (owner_id)
    REFERENCES OWNERS (id)
);

-- Table: JOCKEYS
CREATE TABLE JOCKEYS (
    id integer NOT NULL CONSTRAINT JOCKEYS_pk PRIMARY KEY,
    last_name varchar(50) NOT NULL,
    experience_years integer,
    birth_year integer,
    address varchar(255),
    user_id integer NOT NULL,
    CONSTRAINT JOCKEYS_USERS FOREIGN KEY (user_id)
    REFERENCES USERS (id)
);

-- Table: OWNERS
CREATE TABLE OWNERS (
    id integer NOT NULL CONSTRAINT OWNERS_pk PRIMARY KEY,
    name varchar(30),
    last_name varchar(30),
    middle_name varchar(30),
    address varchar(255),
    phone varchar(20),
    user_id integer NOT NULL,
    CONSTRAINT OWNERS_ak_1 UNIQUE (user_id),
    CONSTRAINT OWNERS_USERS FOREIGN KEY (user_id)
    REFERENCES USERS (id)
);

-- Table: PRIZE_DISTRIBUTION
CREATE TABLE PRIZE_DISTRIBUTION (
    id integer NOT NULL CONSTRAINT PRIZE_DISTRIBUTION_pk PRIMARY KEY,
    race_id integer NOT NULL,
    horse_id integer NOT NULL,
    position integer NOT NULL,
    prize_amount decimal(10,2) NOT NULL,
    distribution_date datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT PRIZE_DISTRIBUTION_RACES FOREIGN KEY (race_id)
    REFERENCES RACES (id),
    CONSTRAINT PRIZE_DISTRIBUTION_HORSES FOREIGN KEY (horse_id)
    REFERENCES HORSES (id),
    CONSTRAINT check_1 CHECK (position IN (1, 2, 3))
);

-- Table: RACES
CREATE TABLE RACES (
    id integer NOT NULL CONSTRAINT RACES_pk PRIMARY KEY,
    race_date datetime NOT NULL,
    race_number integer NOT NULL,
    prize_fund decimal(10,2) NOT NULL
);

-- Table: RACES_HORSES_JOCKEYS
CREATE TABLE RACES_HORSES_JOCKEYS (
    id integer NOT NULL CONSTRAINT RACES_HORSES_JOCKEYS_pk PRIMARY KEY,
    race_id integer NOT NULL,
    horse_id integer NOT NULL,
    jockey_id integer NOT NULL,
    position integer,
    CONSTRAINT RACES_HORSES_JOCKEYS_ak_1 UNIQUE (race_id, horse_id),
    CONSTRAINT RACES_HORSES_JOCKEYS_RACES FOREIGN KEY (race_id)
    REFERENCES RACES (id),
    CONSTRAINT RACES_HORSES_JOCKEYS_HORSES FOREIGN KEY (horse_id)
    REFERENCES HORSES (id),
    CONSTRAINT RACES_HORSES_JOCKEYS_JOCKEYS FOREIGN KEY (jockey_id)
    REFERENCES JOCKEYS (id)
);

-- Table: USERS
CREATE TABLE USERS (
    id integer NOT NULL CONSTRAINT USERS_pk PRIMARY KEY,
    username varchar(50) NOT NULL,
    password varchar(255) NOT NULL,
    role varchar(20) NOT NULL,
    CONSTRAINT USERS_ak_1 UNIQUE (username),
    CONSTRAINT check_1 CHECK (role IN ('admin', 'jockey', 'owner'))
);

-- End of file.

