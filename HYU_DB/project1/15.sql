SELECT T.id, COUNT(*)
FROM Trainer AS T, CatchedPokemon AS CP
WHERE T.id = CP.owner_id
GROUP BY T.id
HAVING COUNT(*) = (
  SELECT MAX(SUB.CNT)
  FROM (
    SELECT COUNT(*) AS CNT
    FROM CatchedPokemon AS CP
    GROUP BY CP.owner_id
  ) AS SUB
)
ORDER BY T.id;