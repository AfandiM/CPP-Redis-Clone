# C++ Redis Clone

This is a learning-oriented project I completed using the Build Your Own organization website.

In tackling this project, I aimed at improving my understanding of data structures and systems programming. I was familiar with Redis functionally through my previous internships, but had no fundamental understanding of its internal workings.

Completing this project allowed me to gain hands-on experience in building an in-memory key-value store from scratch, deepening my knowledge of C++ networking, efficient data storage, and Redis in general.

# Implementation

The clone uses concurrent event-based request handling to handle multiple connections at once. This was implemented using `poll()` to track readiness of multiple sockets. Connections are tracked using a Socket to Connection mapping.

To store entries, the clone uses a hashtable implemented with a linked-list. The hashtable resizes when the load factor is too high.

The clone implements TTL using Heaps and removes expired entries to clear out memory space.

# Usage

The server can be found in the `server.cpp` file.

You can either write your own testing client, or use the one available in client.cpp

The server exposes various commands offered by the actual Redis project. These are:

- **`get (key)`** : Fetch entry with key (key)
- **`set (key, value)`** : Set entry with key as (key) and value as (value)
- **`del (key)`** : Delete entry with key (key)
- **`pexpire (key, timeout_time)`** : Set timeout of entry with key (key) to (timeout_time)
- **`pttl (key)`** : Fetch TTL for entry with key (key)
- **`keys`** : Returns every key in the store
- **`zadd (set, score, value)`** : Adds value (value) with score (score) to sorted set (set)
- **`zrem (set, value)`** : Removes value (value) from sorted set (set)
- **`zscore (set, value)`** : Returns score of value (value) in set (set)
- **`zquery (set, score, value, offset, limit)`** : Find the first entry in zset (set) with score and value (score, value), offset by (offset), and return the (limit) entries thereafter.



