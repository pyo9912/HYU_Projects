SELECT T.name, SUM(CP.level)
FROM Trainer AS T, CatchedPokemon AS CP
WHERE T.id = CP.owner_id
GROUP BY T.id
HAVING SUM(CP.level) = (
  SELECT MAX(SUB.LV)
  FROM (
    SELECT SUM(CP.level) AS LV
    FROM CatchedPokemon AS CP
    GROUP BY CP.owner_id
  ) AS SUB
)
ORDER BY T.name;