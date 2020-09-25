SELECT P.name
FROM CatchedPokemon AS CP, Pokemon AS P
WHERE CP.pid = P.id AND CP.nickname LIKE '% %'
ORDER BY P.name DESC;