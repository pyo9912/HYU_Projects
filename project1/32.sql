SELECT P.name
FROM Pokemon AS P
WHERE P.id NOT IN (
  SELECT P.id
  FROM Pokemon AS P, CatchedPokemon AS CP
  WHERE P.id = CP.pid
  )
 ORDER BY P.name;