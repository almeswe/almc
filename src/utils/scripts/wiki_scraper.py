import requests
from bs4 import BeautifulSoup

url = 'https://en.wikipedia.org/wiki/X86_instruction_listings'
response = requests.get(url, headers = {
	'user-agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/97.0.4692.99 Safari/537.36'
})

if response.status_code != 200:
	raise Exception('Code was not 200...')

soup = BeautifulSoup(response.text, 'html.parser')
table = soup.find('table', {'class': 'wikitable sortable jquery-tablesorter'})

data = []
for tr in soup.find_all('tr'):
	span = tr.find('span', {'class': 'monospaced'})
	if span != None:
		data.append(span.text)

print(f'Find {len(data)} items')
with open('results.txt', 'w', encoding='utf-8') as file:
	for line in data:
		file.write(f'{line}\n')
print('Done')