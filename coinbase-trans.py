import hashlib
def dblsha(data):
 	return hashlib.sha256(hashlib.sha256(data).digest()).digest()

txnlist = [coinbase] + [binascii.a2b_hex(a['data']) for a in template['transactions']]
merklehashes = [dblsha(t) for t in txnlist]
while len(merklehashes) > 1:
	if len(merklehashes) % 2:
	 	merklehashes.append(merklehashes[-1])
 	merklehashes = [dblsha(merklehashes[i] + merklehashes[i + 1]) for i in range(0, len(merklehashes), 2)]
merkleroot = merklehashes[0]
