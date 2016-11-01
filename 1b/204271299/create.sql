-- MYSQL script to generate table w/ appropriate constraints

-- Constraints:
--	- 3 primary key constraints
--	- 6 referential integrity constraints
--	- 3 CHECK constraints
-- ** Write down description of constraints
-- ** for each constraint, demonstrate at least one database modification that
--	violates it in 'violate.sql'

CREATE TABLE Movie
(
	id int NOT NULL,
	title varchar(100) NOT NULL,
	year int NOT NULL,
	rating varchar(10) NOT NULL,
	company varchar(50),
	PRIMARY KEY (id),				-- PRIMARY KEY (1)
	CHECK (year > 1880 AND year < 4000),		-- CHECK (1)
		-- 1880 = first film, 4000 = when flims won't exist anymore ;)
	CHECK (rating = 'G' OR rating = 'PG' OR rating = 'PG-13' OR rating = 'R' OR rating = 'NC-17')					-- CHECK (2)
		-- valid MPAA ratings
)ENGINE = INNODB;		-- invoking innodb storage engine

CREATE TABLE Actor
(
	id int NOT NULL,
	last varchar(20),
	first varchar(20),
	sex varchar(6),
	dob date NOT NULL,
	dod date,
	PRIMARY KEY (id)				-- PRIMARY KEY (2)
)ENGINE = INNODB;

CREATE TABLE Director
(
	id int NOT NULL,
	last varchar(20),
	first varchar(20),
	dob date NOT NULL,
	dod date,
	PRIMARY KEY (id)				-- PRIMARY KEY (3)
)ENGINE = INNODB;

CREATE TABLE MovieGenre
(
	mid int NOT NULL,	-- movie id shared among next 3 tables
	genre varchar(20) NOT NULL,
	FOREIGN KEY (mid) references Movie(id) 		-- FOREIGN KEY (1)
)ENGINE = INNODB;

CREATE TABLE MovieDirector
(
	mid int NOT NULL,
	did int NOT NULL,
	FOREIGN KEY (mid) references Movie(id),		-- FOREIGN KEY (2)
	FOREIGN KEY (did) references Director(id) 	-- FOREIGN KEY (3)
)ENGINE = INNODB;

CREATE TABLE MovieActor
(
	mid int NOT NULL,
	aid int NOT NULL,
	role varchar(50),
	FOREIGN KEY (mid) references Movie(id),		-- FOREIGN KEY (4)
	FOREIGN KEY (aid) references Actor(id)		-- FOREIGN KEY (5)
)ENGINE = INNODB;

-- This table will be empty, will be filled in project 1C
CREATE TABLE Review
(
	name varchar(20),
	time timestamp,
	mid int NOT NULL,
	rating int NOT NULL,
	comment varchar(500),
	FOREIGN KEY (mid) references Movie(id),		-- FOREIGN KEY (6)
	CHECK (rating >= 0 AND rating <= 5)		-- CHECK (3)
		-- from spec: "rating i.e. x out of 5"
)ENGINE = INNODB;

-- For project 1C...
CREATE TABLE MaxPersonID
(
	id int NOT NULL
)ENGINE = INNODB;

CREATE TABLE MaxMovieID
(
	id int NOT NULL
)ENGINE = INNODB;
