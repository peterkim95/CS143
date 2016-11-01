-- MYSQL script that contains THREE select statements w/ english description
--	as comments.

-- Select statement 1:
--	Get names of all actors in move 'Die Another Day'
--	** put names in format: <firstname> <lastname> (separated by space)
--	** Find the ACTOR NAMES that performed in 'Die Another Day' -- found
--		by linking through MovieActor and Movie table.
SELECT CONCAT(a.first,' ',a.last)
FROM Actor a
JOIN MovieActor ma
ON ma.aid = a.id
JOIN Movie m
ON m.id = ma.mid AND m.title = 'Die Another Day';

-- Select statement 2:
--	Get count of all actors who acted in multiple movies
--	** use a cross product with itself and count # of ACTORS who have
--		mismatched movie ids
SELECT COUNT(DISTINCT m.aid)
FROM MovieActor m, MovieActor m2
WHERE m.aid = m2.aid AND m.mid != m2.mid;

-- Select statement 3: 
--	Query that you come up with.
-- I want to select the first last name and DOB of the ACTORS who starred in
--	more than 25 movies. I achieve this by writing a subquery that grabs
--	the distinct number of movies each actor acted in.
SELECT CONCAT(a.first,' ',a.last),
       a.dob
FROM Actor a
WHERE 25 < 
	(	
	SELECT COUNT(DISTINCT m.mid) 
	FROM MovieActor m
	WHERE m.aid = a.id   
	);
