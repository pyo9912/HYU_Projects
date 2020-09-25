SELECT T.name 
FROM Trainer AS T, CatchedPokemon AS CP
WHERE T.id = CP.owner_id
AND CP.pid IN (
  SELECT CP.pid
  FROM Trainer AS T, CatchedPokemon AS CP
  WHERE T.id = CP.owner_id
  GROUP BY T.id, CP.pid
  HAVING COUNT(*) >= 2
  )
AND T.id IN (
  SELECT T.id
  FROM Trainer AS T, CatchedPokemon AS CP
  WHERE T.id = CP.owner_id
  GROUP BY T.id, CP.pid
  HAVING COUNT(*) >= 2
  )
ORDER BY T.name;