SELECT P.name
FROM Pokemon AS P, Evolution AS E
WHERE E.before_id > E.after_id AND P.id = E.before_id
ORDER BY P.name;