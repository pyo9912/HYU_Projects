SELECT DISTINCT P.name, P.type
FROM Pokemon AS P, CatchedPokemon AS CP
WHERE P.id = CP.pid AND CP.level >= 30
ORDER BY P.name;