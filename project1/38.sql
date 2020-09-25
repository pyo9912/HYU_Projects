SELECT P.name
FROM Pokemon AS P, Evolution AS E
WHERE P.id = E.after_id
AND P.id NOT IN (
  SELECT before_id
  FROM Evolution
  )
ORDER BY P.name;