import requests
import json
import csv
import numpy as np
from requests_oauthlib import OAuth2Session

# https://github.com/login/oauth/authorize?response_type=code&client_id=f31165013adac0da36ed&state=DKq8zpOEQ9LruJY7qc3BZIJk5SsLNc

# https://gigamesh.eu/?page=home&code=b97bd16717cad87f1478&state=N4OtsRRH4rq698sDXJYX2OSiFrr7ml


def main():

	# ---------- authenticate user ----------

	# ... with python requests

	preset = False

	# github data
	client_id = 'f31165013adac0da36ed'
	client_secret = '32d6f2a7939c1b40cae13c20a36d4cd32942e60d'
	authorization_base_url = 'https://github.com/login/oauth/authorize'
	token_url = 'https://github.com/login/oauth/access_token'
	authorization_url = 'https://github.com/login/oauth/authorize?response_type=code&client_id=f31165013adac0da36ed&state=DKq8zpOEQ9LruJY7qc3BZIJk5SsLNc'
	
	if preset:
		github = OAuth2Session(client_id)

		# Redirect user to GitHub for authorization
		authorization_url, state = github.authorization_url(authorization_base_url)
		print('Please go here and authorize,', authorization_url)

		# Get the authorization verifier code from the callback url
		redirect_response = input('Paste the full redirect URL here:')

		# Fetch the access token
		github.fetch_token(token_url, client_secret=client_secret,
		     authorization_response=redirect_response)

		# Fetch a protected resource, i.e. user profile
		r = github.get('https://api.github.com/user')
		print(r.content)

	else:
		s = requests.Session()
		s.get(authorization_base_url) # needed for allowing cookies
		p = {'client_id': client_id, 'response_type': 'code', 'redirect_uri': 'https://gigamesh.eu/?page=home'}
		r = s.get(authorization_base_url, params=p) # get request needed 
		#r = s.get(authorization_url) 

		if(r):
			print(str(r.status_code) + " - Authetication successfully.")
		else:
			print(str(r.status_code) + " - Error while authenticating!")

		#response = r.json()
		#print(response['url'])
		print(r.url)
		#print()
		print(r.headers.get('code'))

	return 0



if __name__== "__main__":
	main()


