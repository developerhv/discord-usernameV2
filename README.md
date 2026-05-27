# discord-usernameV2

Simple way to grab a user's Discord username.

Workflow:

1. Searches for Discord's `scope_v3.json` located in:
   `AppData\Roaming\discord\sentry\scope_v3.json`

2. If the file doesn't exist or doesn't contain a valid username, it falls back to scanning Discord's memory for:

   `"username":"`

3. Extracts the username, cleans it up, and returns it.

## Why?

Most implementations rely on:
- Discord APIs
- Hardcoded offsets
- Version-specific structures
- External libraries

This doesn't.

The filesystem method works for the majority of users and is extremely fast. The memory fallback dynamically searches for the username pattern rather than relying on offsets, making it far more resilient across Discord updates.

## Features

- No Discord API
- No hardcoded offsets
- No signatures to update
- No external dependencies
- No DLLs
- No injection
- Fast filesystem lookup
- Dynamic memory fallback

## Notes

The memory scanner walks readable committed pages inside `Discord.exe`, searches for the JSON username pattern, validates the result, filters obvious garbage strings, and returns the first valid username found.
