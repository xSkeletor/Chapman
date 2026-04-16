# spotify
This repository contains a spotify.py program which uses the Spotify API:
* The client asks the user to input the name of 2 Spotify artists.
* It then authenticates with Spotify to return a 10 min access token using the clients id and secret.
* Then it gets the gets the artists id based on the user input provided, and searchs for at least 50 albums for each artist.
* Each albumn is then printed out to the console in a readable format.
* The program shows which artist has more albums.

## Identifying Information
* Name: Dennis Fomichev
* Student ID: 2470131
* Email: fomichev@chapman.edu
* Course: CPSC 353
* Assignment: PA03 Spotify

## Source Files  
* spotify.py

## References
* N/A

## Known Errors
* N/A

## Build Instructions
* pip install requests

## Execution Instructions
* spotify.py < spotify-input