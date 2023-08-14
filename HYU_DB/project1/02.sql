SELECT P.name
FROM Pokemon AS P, (
  SELECT PK.type
  FROM (
    SELECT type, COUNT(*) AS C
    FROM Pokemon
    GROUP BY type
  )AS PK,
  (
    SELECT SUB1.C AS N
    FROM (
      SELECT COUNT(*) AS C
      FROM Pokemon
      GROUP BY type
      ORDER BY COUNT(*) DESC LIMIT 2
    ) AS SUB1
  ) AS SUB2
  WHERE PK.C = SUB2.N
)AS SUB3
WHERE P.type = SUB3.type
ORDER BY P.name ASC;