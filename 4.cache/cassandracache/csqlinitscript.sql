cqlsh> CREATE KEYSPACE IF NOT EXISTS testtask WITH REPLICATION = { 'class' : 'NetworkTopologyStrategy', 'datacenter1' : 3 };
cqlsh> CREATE TABLE testtask.cache (id UUID PRIMARY KEY, key text, value text);
cqlsh> INSERT INTO testtask.cache (id, key, value) VALUES (5b6962dd-3f90-4c93-8f61-eabfa4a803e2, 'one','val1');
cqlsh> INSERT INTO testtask.cache (id, key, value) VALUES (5b6962dd-3f90-4c93-8f61-eabfa4a803e3, 'two','val2');
cqlsh> INSERT INTO testtask.cache (id, key, value) VALUES (5b6962dd-3f90-4c93-8f61-eabfa4a803e4, 'three','val3');

