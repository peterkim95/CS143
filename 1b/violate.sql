-- list of SQL modification statements (INSERT/DELETE/UPDATE) that violate
--	each of the constraints that I set 

-- Primary key violations:
-- ** The idea behind these faulty insert statements is that there already
--	exists a tuple with the primary key of the tuple I try to insert...

-- id 92 already exists
INSERT INTO Movie VALUES (92,'scott',2000,'R','dirty');
-- ERROR 1062 (23000): Duplicate entry '92' for key 'PRIMARY'

-- id 19 already exists
INSERT INTO Actor VALUES (19,'shi','scott','Male',2016-10-10,2070-10-10);
-- ERROR 1062 (23000): Duplicate entry '19' for key 'PRIMARY'

-- id 16 already exists
INSERT INTO Director VALUES (16,'kim','peter',2000-10-10,3000-10-10);
-- ERROR 1062 (23000): Duplicate entry '16' for key 'PRIMARY'




-- Foreign key violations:
-- ** The idea behind these faulty insert statements is that I try to insert
--	a non-existent ID into the MovieGenre/MovieDirector/MovieActor/Review
--	tables. The foreign key constraint will try to check if the id that I
--	insert into the table exists in the table it refers to.

-- id 10 doesn't exist in Movie 
INSERT INTO MovieGenre VALUES (10,'edgy');
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143'.'MovieGenre', CONSTRAINT 'MovieGenre_ibfk_1' FOREIGN KEY('mid') REFERENCES 'Movie' ('id'))

-- id 10 doesn't exist in Movie
INSERT INTO MovieDirector VALUES (10,16);
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143'.'MovieDirector', CONSTRAINT 'MovieDirector_ibfk_1' FOREIGN KEY('mid') REFERENCES 'Movie' ('id'))

-- id 1 doesn't exist in Director
INSERT INTO MovieDirector VALUES (16,1);
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143'.'MovieDirector', CONSTRAINT 'MovieDirector_ibfk_2' FOREIGN KEY('did') REFERENCES 'Director' ('id'))

-- id 10 doesn't exist in Movie
INSERT INTO MovieActor VALUES(10,1,'blah');
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143'.'MovieActor', CONSTRAINT 'MovieActor_ibfk_1' FOREIGN KEY('mid') REFERENCES 'Movie' ('id'))

-- id 2 doesn't exist in Actor
INSERT INTO MovieActor VALUES(16,2,'blah');
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143'.'MovieActor', CONSTRAINT 'MovieActor_ibfk_2' FOREIGN KEY('aid') REFERENCES 'Actor' ('id'))

-- id 10 doesn't exist in Movie
INSERT INTO Review VALUES('asf','10-10-2000 00:00:00',10,5,'movie was guht');
-- ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails ('CS143'.'Review', CONSTRAINT 'Review_ibfk_1' FOREIGN KEY('mid') REFERENCES 'Movie' ('id'))



-- Check violations (not tested):
-- 	Check 1: I violate fact that 1880 < year < 4000
INSERT INTO Movies VALUES (111111,'notamovie',1000,'G','notacompany');
 
--	Check 2: I violate fact that 'rating' is not one of the accepted answers
INSERT INTO Movies VALUES (111111,'notamovie',2000,'X','notacompany');

--	Check 3: I violate fact that 'rating' is not between 0 and 5
INSERT INTO Review VALUES ('notamovie','01-01-2000 00:00:00',2,6,'this movie kind of sucked...');
