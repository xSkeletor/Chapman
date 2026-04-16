import base64
import json
from requests import post, get

'''
    Spotify Program
    This Python program asks the user to input the name of 2 Spotify artists.
    It then authenticates with Spotify to return a 10 min access token using the clients id and secret.
    Then it gets the gets the artists id based on the user input provided, and searchs for at least 50 albums for each artist
    Each albumn is then printed out to the console in a readable format.
    The program shows which artist has more albums.

    @author: Dennis Fomichev
        email: fomichev@chapman.edu
        date: 03/07/2025
    @version: 1.0
'''

# ID and secret from Spotify Developer portal
client_id = "aa9990c1e0de4025969fb408f146bc38"
client_secret = "284d2ec437164b9a8eb2cd21395c7621"

# use your client_id and client secret to get a token
# the token will be valid for 10 minutes
# you can use it to access the Spotify API
def get_token():
    auth_string = client_id + ":" + client_secret
    auth_bytes = auth_string.encode("utf-8")
    auth_base64 = str(base64.b64encode(auth_bytes), "utf-8")

    url = "https://accounts.spotify.com/api/token"
    headers = {
        "Authorization": "Basic " + auth_base64,
        "Content-Type": "application/x-www-form-urlencoded",
    }

    data = {"grant_type": "client_credentials"}
    result = post(url, headers=headers, data=data)
    json_result = json.loads(result.content)
    token = json_result["access_token"]

    return token

# you will use this header in functions that retrieve info from the API
def get_auth_header(token):
    return {"Authorization": "Bearer " + token}

# this function returns an artists record that includes the artist_id
def search_for_artist(token, artist_name):
    url = "https://api.spotify.com/v1/search"
    query = f"?q={artist_name}&type=artist&limit=1"
    query_url = url + query
    headers = get_auth_header(token)
    result = get(query_url, headers=headers)
    json_result = json.loads(result.content)["artists"]["items"]

    if len(json_result) == 0:
        print("no artist with this name exists...")
        return None

    return json_result[0]

# Returns at most 50 albumns for a given artist based in their provided id
def get_albums_by_artist(token, artist_id):
    url = f"https://api.spotify.com/v1/artists/{artist_id}/albums?limit=50"
    headers = get_auth_header(token)
    result = get(url, headers=headers)
    json_result = json.loads(result.content)["items"]

    return json_result

# The main program starts here and gets the token
token = get_token()

# Display the purpose of the program
print("This program will print a list of albums from the 2 artists you select and show who has more.")

# Get the name of the artists from the user
artist1_name = input("Enter the name of your first artist: ")
artist2_name = input("Enter the name of your second artist: ")

# Get the artist id for the first artist
result1 = search_for_artist(token, artist1_name)
artist1_id = result1["id"]
print("The artist_id for", artist1_name, "is", artist1_id)

# Get the artist id for the second artist
result2 = search_for_artist(token, artist2_name)
artist2_id = result2["id"]
print("The artist_id for", artist2_name, "is", artist2_id)

# Get each artists albums
albums1 = get_albums_by_artist(token, artist1_id)
albums2 = get_albums_by_artist(token, artist2_id)

print("\n", artist1_name, "albums:")

# Check if the amount of albumns returned for artist1 is 50, if so, say that there might be more
if len(albums1) == 50:
    print(f"\n {artist1_name} has at least 50 albums, only the first 50 will be displayed\n")

# Loop through the albumns and print out each name
for idx, album in enumerate(albums1):
    print(f"{idx + 1}. {album['name']}")

print("\n", artist2_name, "albums:")

# Check if the amount of albumns returned for artist1 is 50, if so, say that there might be more
if len(albums2) == 50:
    print(f"\n {artist2_name} has at least 50 albums, only the first 50 will be displayed\n")

# Loop through the albumns and print out each name
for idx, album in enumerate(albums2):
    print(f"{idx + 1}. {album['name']}")

# Print out which artist has more albumns
if len(albums1) > len(albums2):
    print(f"\n{artist1_name} has more albums than {artist2_name}")
else:
    print(f"\n{artist2_name} has more albums than {artist1_name}")
