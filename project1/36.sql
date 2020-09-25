SELECT T.name
FROM Trainer AS T, CatchedPokemon AS CP, Pokemon AS P, Evolution AS E
WHERE T.id = CP.owner_id AND CP.pid = P.id AND P.id = E.after_id
AND E.after_id NOT IN (
  SELECT E.before_id
  FROM Evolution AS E
  )
ORDER BY T.name;