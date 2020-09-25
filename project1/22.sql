SELECT P.type, COUNT(*) AS '#Pokemon'
FROM Pokemon AS P
GROUP BY P.type
ORDER BY COUNT(*);