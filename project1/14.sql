SELECT P.name
FROM Pokemon AS P, Evolution AS E
WHERE P.type = 'Grass' AND P.id = E.before_id;