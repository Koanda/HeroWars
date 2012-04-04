/* packet-Plol.c
 * Routines for League of Legends GC dissection
 * Copyright 2011, Intline9 <intline9@gmail.com>
 *
 * $Id$
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * Copied from WHATEVER_FILE_YOU_USED (where "WHATEVER_FILE_YOU_USED"
 * is a dissector file; if you just copied this from README.developer,
 * don't bother with the "Copied from" - you don't even need to put
 * in a "Copied from" if you copied an existing dissector, especially
 * if the bulk of the code in the new dissector is your code)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "lol.h"

/* Special CRC */
static guint16 crcsum(guint16 crc, const guint8* message, guint length);

static gboolean haveCrc = FALSE;
gboolean initialized = FALSE;

char *myKey = "BQ/rlw1ivzbg7IG/6/iXRQ==";
byte key[16];
gboolean isKey = FALSE;

/* Handlers */
static gint ett_lol = -1;
static gint ett_enet = -1;
static int proto_lol = -1;
static int proto_enet = -1;
static dissector_handle_t lol_handle;

/* Structure */
static GHashTable *fragmentTable = NULL;
static GHashTable *reassembledTable = NULL;
/* Enet */
static int hf_enet_header = -1;
static int hf_enet_peerId = -1;
static int hf_enet_sentTime = -1;
static int hf_enet_flags = -1;
static int hf_enet_checksum = -1;

static const value_string commands[] = {
	{ 0, "ENET_PROTOCOL_COMMAND_NONE" },
	{ 1, "ENET_PROTOCOL_COMMAND_ACKNOWLEDGE" },
	{ 2, "ENET_PROTOCOL_COMMAND_CONNECT" },
	{ 3, "ENET_PROTOCOL_COMMAND_VERIFY_CONNECT" },
	{ 4, "ENET_PROTOCOL_COMMAND_DISCONNECT" },
	{ 5, "ENET_PROTOCOL_COMMAND_PING" },
	{ 6, "ENET_PROTOCOL_COMMAND_SEND_RELIABLE" },
	{ 7, "ENET_PROTOCOL_COMMAND_SEND_UNRELIABLE" },
	{ 8, "ENET_PROTOCOL_COMMAND_SEND_FRAGMENT" },
	{ 9, "ENET_PROTOCOL_COMMAND_SEND_UNSEQUENCED" },
	{ 10, "ENET_PROTOCOL_COMMAND_BANDWIDTH_LIMIT" },
	{ 11, "ENET_PROTOCOL_COMMAND_THROTTLE_CONFIGURE" },
	{ 12, "ENET_PROTOCOL_COMMAND_SEND_UNRELIABLE_FRAGMENT" },
};
static int hf_enet_commandHeader = -1;
static int hf_enet_command = -1;
static int hf_enet_channelId = -1;
static int hf_enet_sequenceNumber = -1;

static int hf_enet_startSequence = -1;
static int hf_enet_dataLength = -1;
static int hf_enet_fragmentCount = -1;
static int hf_enet_fragmentNo = -1;
static int hf_enet_totalLength = -1;
static int hf_enet_fragmentOffset = -1;


static int hf_lol_packet = -1;
static int hf_lol_length = -1;
static int hf_lol_command = -1;

/* Global sample preference ("controls" display of numbers) */
guint8 *gPREF_KEY = NULL;
guint gPREF_PORT = 5000;


static int hf_msg_fragments = -1;
static int hf_msg_fragment = -1;
static int hf_msg_fragment_overlap = -1;
static int hf_msg_fragment_overlap_conflicts = -1;
static int hf_msg_fragment_multiple_tails = -1;
static int hf_msg_fragment_too_long_fragment = -1;
static int hf_msg_fragment_error = -1;
static int hf_msg_fragment_count = -1;
static int hf_msg_reassembled_in = -1;
static int hf_msg_reassembled_length = -1;
static gint ett_msg_fragment = -1;
static gint ett_msg_fragments = -1;

static const fragment_items msg_frag_items = {
	/* Fragment subtrees */
	&ett_msg_fragment,
	&ett_msg_fragments,
	/* Fragment fields */
	&hf_msg_fragments,
	&hf_msg_fragment,
	&hf_msg_fragment_overlap,
	&hf_msg_fragment_overlap_conflicts,
	&hf_msg_fragment_multiple_tails,
	&hf_msg_fragment_too_long_fragment,
	&hf_msg_fragment_error,
	&hf_msg_fragment_count,
	/* Reassembled in field */
	&hf_msg_reassembled_in,
	/* Reassembled length field */
	&hf_msg_reassembled_length,
	/* Tag */
	"Message fragments"
};

void proto_register_lol(void)
{
	module_t *lol_module;
	
	static hf_register_info hf_enet[] = {
		{ &hf_enet_header,
			{"Enet Header", "enet.header",
			FT_NONE, BASE_NONE,
			NULL, 0x0,
			NULL, HFILL}
		},

		{ &hf_enet_peerId,
			{"Peer ID", "enet.header.peerid",
			FT_UINT16, BASE_DEC,
			NULL, 0x0,
			NULL, HFILL}
		},
		{ &hf_enet_flags,
		{"Flags", "enet.header.flags",
			FT_UINT16, BASE_HEX,
			NULL, 0x0,
			NULL, HFILL}
		},
		{ &hf_enet_sentTime,
			{"Send Time", "enet.header.senttime",
			FT_UINT16, BASE_DEC,
			NULL, 0x0,
			NULL, HFILL}
		},
		{ &hf_enet_checksum,
		{"Checksum", "enet.header.checksum",
			FT_UINT32, BASE_HEX,
			NULL, 0x0,
			NULL, HFILL}
		},
		/* Command header */
		{ &hf_enet_commandHeader,
			{"Enet Command", "enet.cheader",
			FT_NONE, BASE_NONE,
			NULL, 0x0,
			NULL, HFILL}
		},
		{ &hf_enet_command,
			{"Command", "enet.cheader.command",
			FT_UINT8, BASE_DEC,
			VALS(commands), 0x0F,
			NULL, HFILL}
		},
		{ &hf_enet_channelId,
			{"Channel ID", "enet.cheader.channelid",
			FT_UINT8, BASE_DEC,
			NULL, 0x0,
			NULL, HFILL}
		},
		{ &hf_enet_sequenceNumber,
		{"Sequence number", "enet.cheader.seqno",
			FT_UINT16, BASE_DEC,
			NULL, 0x0,
			NULL, HFILL}
		},
		{ &hf_enet_startSequence,
		{"Start sequence", "enet.cheader.startseq",
			FT_UINT16, BASE_DEC,
			NULL, 0x0,
			NULL, HFILL}
		},
		{ &hf_enet_dataLength,
		{"Data length", "enet.cheader.datalen",
			FT_UINT16, BASE_DEC,
			NULL, 0x0,
			NULL, HFILL}
		},
		{ &hf_enet_fragmentCount,
		{"Fragment count", "enet.cheader.frag.count",
			FT_UINT32, BASE_DEC,
			NULL, 0x0,
			NULL, HFILL}
		},
		{ &hf_enet_fragmentNo,
			{"Fragment number", "enet.cheader.frag.no",
			FT_UINT32, BASE_DEC,
			NULL, 0x0,
			NULL, HFILL}
		},
		{ &hf_enet_totalLength,
			{"Total length", "enet.cheader.frag.length",
			FT_UINT32, BASE_DEC,
			NULL, 0x0,
			NULL, HFILL}
		},
		{ &hf_enet_fragmentOffset,
			{"Fragment offset", "enet.cheader.frag.offset",
			FT_UINT32, BASE_DEC,
			NULL, 0x0,
			NULL, HFILL}
		},

		/* Send structs */
		{ &hf_lol_packet,
			{"Data", "lol.data",
			FT_NONE, BASE_NONE,
			NULL, 0x0,
			NULL, HFILL}
		},
		{ &hf_lol_length,
			{"Data length", "lol.length",
			FT_UINT16, BASE_DEC,
			NULL, 0x0,
			NULL, HFILL}
		},
		{ &hf_lol_command,
			{"Command", "lol.cmd",
			FT_UINT8, BASE_HEX,
			NULL, 0x0,
			NULL, HFILL}
		},

		/* Fragments */
		{&hf_msg_fragments,
		{"Message fragments", "msg.fragments",
		FT_NONE, BASE_NONE, NULL, 0x00, NULL, HFILL } },
		{&hf_msg_fragment,
		{"Message fragment", "msg.fragment",
		FT_FRAMENUM, BASE_NONE, NULL, 0x00, NULL, HFILL } },
		{&hf_msg_fragment_overlap,
		{"Message fragment overlap", "msg.fragment.overlap",
		FT_BOOLEAN, 0, NULL, 0x00, NULL, HFILL } },
		{&hf_msg_fragment_overlap_conflicts,
		{"Message fragment overlapping with conflicting data",
		"msg.fragment.overlap.conflicts",
		FT_BOOLEAN, 0, NULL, 0x00, NULL, HFILL } },
		{&hf_msg_fragment_multiple_tails,
		{"Message has multiple tail fragments",
		"msg.fragment.multiple_tails",
		FT_BOOLEAN, 0, NULL, 0x00, NULL, HFILL } },
		{&hf_msg_fragment_too_long_fragment,
		{"Message fragment too long", "msg.fragment.too_long_fragment",
		FT_BOOLEAN, 0, NULL, 0x00, NULL, HFILL } },
		{&hf_msg_fragment_error,
		{"Message defragmentation error", "msg.fragment.error",
		FT_FRAMENUM, BASE_NONE, NULL, 0x00, NULL, HFILL } },
		{&hf_msg_fragment_count,
		{"Message fragment count", "msg.fragment.count",
		FT_UINT32, BASE_DEC, NULL, 0x00, NULL, HFILL } },
		{&hf_msg_reassembled_in,
		{"Reassembled in", "msg.reassembled.in",
		FT_FRAMENUM, BASE_NONE, NULL, 0x00, NULL, HFILL } },
		{&hf_msg_reassembled_length,
		{"Reassembled length", "msg.reassembled.length",
		FT_UINT32, BASE_DEC, NULL, 0x00, NULL, HFILL } },
	};

	/* Setup protocol subtree array */
	static gint *ett[] = {
		&ett_enet,
		&ett_lol,
		&ett_msg_fragment,
		&ett_msg_fragments
	};

	proto_enet = proto_register_protocol(
		"Enet",								/* name */
		"Enet",								/* short name */
		"enet"								/* abbrev */
	);

	proto_lol = proto_register_protocol(
		"League of Legends",				/* name */
		"LoL",								/* short name */
		"lol"								/* abbrev */
	);
	
	fragment_table_init(&fragmentTable);
	reassembled_table_init(&reassembledTable);

	proto_register_field_array(proto_enet, hf_enet, array_length(hf_enet));
	proto_register_subtree_array(ett, array_length(ett));
	
	/* Preferences */
	lol_module = prefs_register_protocol(proto_lol, proto_reg_handoff_lol);
	

	gPREF_KEY = (guint8*)g_malloc(32 * sizeof(guint8)); memset(gPREF_KEY, 0, 32 * sizeof(guint8));
	prefs_register_string_preference(lol_module,
		"lolpref.key",
	    "Blowfish key",
		"The key needed for this game session to decrypt the packets.",
		&gPREF_KEY
	);
	
	prefs_register_uint_preference(lol_module,
		"lolpref.port",
		"LoL GC port",
		"Port to listen on for this game session.",
		10,
		&gPREF_PORT
	);
	
	/* Start the watchdog thread */
	start_listener();
	atexit(stop_listener);
}

void set_single_port(guint port)
{
	int i;
	for(i = LOL_PORT_MIN; i < LOL_PORT_MAX; i++)
		dissector_delete_uint("udp.port", i, lol_handle);
	dissector_add_uint("udp.port", port, lol_handle);
}

void proto_reg_handoff_lol(void)
{
	int i;
	static int currentPort;
	
	if(!initialized)
	{
		lol_handle = create_dissector_handle(dissect_lol, proto_lol);

		//Set defaults
		OutputDebugStringA("Settings defaults");
		base64_init_decodestate(&base64);
		base64_decode_block(myKey, (int)strlen(myKey), key, &base64);
		isKey = TRUE;
		OutputDebugStringA(myKey);

		initialized = TRUE;
	}
	
	for(i = LOL_PORT_MIN; i < LOL_PORT_MAX; i++)
		dissector_add_uint("udp.port", i, lol_handle);
}

static tvbuff_t *getDecryptedTvb(packet_info *pinfo, tvbuff_t *tvb, guint tvbOffset, guint length)
{
	guint i = 0, encryptedLength = 0;
	BLOWFISH_context c;
	guchar *decrypted, *packetData;
	tvbuff_t *new_tvb;
	char buf[255];

	encryptedLength = length-(length%8); //Decrypt 8 byte blocks
	
	decrypted = (guchar*)g_malloc(length);
	packetData = (guchar*)tvb_get_ptr(tvb, tvbOffset, length);

	bf_setkey(&c, key, (unsigned)16);

	while(i < encryptedLength)
	{
		sprintf_s(buf, 255, "Decrypt block, startOffset: %i, encOffset: %i, length: %i, encLength: %i", tvbOffset, i, length, encryptedLength);
		//OutputDebugString(buf);
		decrypt_block(&c, (byte*)&decrypted[i], (byte*)&packetData[i]);
		i+=8;
	}
	memcpy(&decrypted[i], &packetData[i], (length%8)); //Copy remained unencrypted bytes

	new_tvb = tvb_new_real_data(decrypted, length, length);
	tvb_set_child_real_data_tvbuff(tvb, new_tvb);
	add_new_data_source(pinfo, new_tvb, "Decrypted");
	return new_tvb;
}

static guint dissect_lolPacket(tvbuff_t *tvb, packet_info *pinfo, proto_tree *enetTree, guint dataLength, guint offset)
{
	proto_tree *lolTree = NULL;
	proto_item *lolNode = NULL, *packetItem = NULL;
	tvbuff_t *newTvb = NULL;
	guint newOffset;
	guint16 encLength = dataLength-(dataLength%8);
	
	/* Create a sub node for this package */
	lolNode = proto_tree_add_item(enetTree, proto_lol, tvb, offset, dataLength, FALSE);
	lolTree = proto_item_add_subtree(lolNode, ett_lol);
	
	if(dataLength >= 8)
	{
		newTvb = getDecryptedTvb(pinfo, tvb, offset, dataLength);
		if(newTvb != NULL)
			newOffset = 0;
		else
			goto normal;
	}	
	else
	{
		normal:
		newOffset = offset;
		newTvb = tvb;
	}

	proto_tree_add_item(lolTree, hf_lol_command, newTvb, newOffset, 1, ENC_NA); newOffset+= 1;
	packetItem = proto_tree_add_item(lolTree, hf_lol_packet, newTvb, newOffset, dataLength-1, ENC_NA); newOffset+= dataLength-1;

	offset += dataLength;
	return offset;
}

static guint dissect_enet_dissectCommandHeader(tvbuff_t *tvb, proto_item **subNode, proto_tree *enetTree, ENetProtocol *command, guint offset)
{
	proto_item *item = NULL, *commandHeader = NULL;
	item = proto_tree_add_item(enetTree, hf_enet_commandHeader, tvb, offset, 4, FALSE);
	commandHeader = proto_item_add_subtree(item, ett_enet);
	*subNode = commandHeader;

	proto_tree_add_item(commandHeader, hf_enet_command, tvb, offset, 1, ENC_NA); offset += 1;
	proto_tree_add_item(commandHeader, hf_enet_channelId, tvb, offset, 1, ENC_NA); offset += 1;
	proto_tree_add_item(commandHeader, hf_enet_sequenceNumber, tvb, offset, 2, ENC_BIG_ENDIAN); offset += 2;

	return offset;
}

static guint dissect_enet_sendReliable(tvbuff_t *tvb, packet_info *pinfo, proto_tree *enetTree, ENetProtocol *command, guint offset)
{
	proto_item *enetCommandHeader = NULL;
	guint dataLength = 0;

	offset = dissect_enet_dissectCommandHeader(tvb, &enetCommandHeader, enetTree, command, offset);
	
	dataLength = (guint16)tvb_get_ntohs(tvb, offset);
	proto_tree_add_item(enetCommandHeader, hf_enet_dataLength, tvb, offset, 2, ENC_BIG_ENDIAN); offset += 2;
	
	offset = dissect_lolPacket(tvb, pinfo, enetTree, dataLength, offset);
	return offset;
}



static guint dissect_enet_sendFragment(tvbuff_t *tvb, packet_info *pinfo, proto_tree *enetTree, ENetProtocol *command, guint offset)
{
	fragment_data *frag;
	gboolean state = pinfo->fragmented;
	proto_item *enetCommandHeader = NULL;
	fragment_data *fragMsg = NULL;
	tvbuff_t* new_tvb = NULL;
	guint dataLength, sequenceId, fragCount, fragNo, totalLength;
	gboolean lastFragment;
	char buf[255];
	offset = dissect_enet_dissectCommandHeader(tvb, &enetCommandHeader, enetTree, command, offset);

	sequenceId = tvb_get_ntohs(tvb, offset);
	dataLength = tvb_get_ntohs(tvb, offset+2);
	fragCount = tvb_get_ntohl(tvb, offset+4)-1;
	fragNo = tvb_get_ntohl(tvb, offset+8);
	totalLength = tvb_get_ntohl(tvb, offset+12);
	lastFragment = (fragNo == fragCount);

	
	proto_tree_add_item(enetCommandHeader, hf_enet_startSequence, tvb, offset, 2, ENC_BIG_ENDIAN); offset += 2;
	proto_tree_add_item(enetCommandHeader, hf_enet_dataLength, tvb, offset, 2, ENC_BIG_ENDIAN); offset += 2;
	proto_tree_add_item(enetCommandHeader, hf_enet_fragmentCount, tvb, offset, 4, ENC_BIG_ENDIAN); offset += 4;
	proto_tree_add_item(enetCommandHeader, hf_enet_fragmentNo, tvb, offset, 4, ENC_BIG_ENDIAN); offset += 4;
	proto_tree_add_item(enetCommandHeader, hf_enet_totalLength, tvb, offset, 4, ENC_BIG_ENDIAN); offset += 4;
	proto_tree_add_item(enetCommandHeader, hf_enet_fragmentOffset, tvb, offset, 4, ENC_BIG_ENDIAN); offset += 4;

	proto_tree_add_item(enetCommandHeader, hf_lol_packet, tvb, offset, dataLength, ENC_NA);

	sprintf_s(buf, 255, "Fragment(%i), length: %i(%i), seqId: %i, fragCount: %i, fragNo: %i, totalLength: %i, isLast: %i", pinfo->fragmented, dataLength,  tvb_length_remaining(tvb, offset), sequenceId, fragCount, fragNo, totalLength, lastFragment);
	OutputDebugString(buf);
	pinfo->fragmented = TRUE;
	printf("SO ... FUCKED UP");
	fragMsg = fragment_add_seq_check(tvb, offset, pinfo, sequenceId, fragmentTable, reassembledTable, fragNo,  tvb_length_remaining(tvb, offset), !lastFragment);
	frag = fragment_get(pinfo, sequenceId, fragmentTable);
	sprintf_s(buf, 255, "FragMsg: %X, len: %i", frag, fragment_get_tot_len(pinfo, sequenceId, fragmentTable));
	printf("SO ... FUCKED UP");
	OutputDebugString(buf);

	if(!lastFragment)
	{
		col_append_str(pinfo->cinfo, COL_INFO," (fragments)");
	}
	else
	{
		OutputDebugString("LAST FRAGMENT BICH!!");
		col_append_str(pinfo->cinfo, COL_INFO," (Last fragment)");
	}
	new_tvb = process_reassembled_data(tvb, offset, pinfo, "Reassembled Message", fragMsg, &msg_frag_items, NULL, enetTree);

	if (fragMsg) { /* Reassembled */
		col_append_str(pinfo->cinfo, COL_INFO,
			" (Message Reassembled)");
	} else { /* Not last packet of reassembled Short Message */
		col_append_fstr(pinfo->cinfo, COL_INFO,
			" (Message fragment %u)", fragNo);
	}

	if (new_tvb) { /* take it all */
		//next_tvb = new_tvb;
	} else { /* make a new subset */
		//next_tvb = tvb_new_subset(tvb, offset, -1, -1);
	}

	pinfo->fragmented = state;
	offset = dissect_lolPacket(tvb, pinfo, enetTree, dataLength, offset);
	return offset;
}

static guint dissect_command(tvbuff_t *tvb, packet_info *pinfo, proto_tree *enetTree, unsigned char *enetData, guint offset)
{
	ENetProtocol * command;
	guint length = tvb_length(tvb);

	command = (ENetProtocol *)( enetData+offset);
	switch (command->header.command & ENET_PROTOCOL_COMMAND_MASK)
	{
		case ENET_PROTOCOL_COMMAND_ACKNOWLEDGE:
			col_append_str(pinfo->cinfo, COL_INFO, "Ack, ");
			break;
		case ENET_PROTOCOL_COMMAND_CONNECT:
			col_append_str(pinfo->cinfo, COL_INFO, "Connect, ");
			break;
		case ENET_PROTOCOL_COMMAND_VERIFY_CONNECT:
			col_append_str(pinfo->cinfo, COL_INFO, "Check Connect, ");
			break;
		case ENET_PROTOCOL_COMMAND_DISCONNECT:
			col_append_str(pinfo->cinfo, COL_INFO, "Disconnect, ");
			break;

		case ENET_PROTOCOL_COMMAND_PING:
			col_append_str(pinfo->cinfo, COL_INFO, "Ping, ");
			break;

		case ENET_PROTOCOL_COMMAND_SEND_RELIABLE:
			offset = dissect_enet_sendReliable(tvb, pinfo, enetTree, command, offset);
			if(length >= offset+sizeof(ENetProtocolCommandHeader))
				offset = dissect_command(tvb, pinfo, enetTree, enetData, offset);
			col_append_str(pinfo->cinfo, COL_INFO, "Send reliable, ");
			break;

		case ENET_PROTOCOL_COMMAND_SEND_UNRELIABLE:
			col_append_str(pinfo->cinfo, COL_INFO, "Send unreliable, ");
			break;

		case ENET_PROTOCOL_COMMAND_SEND_UNSEQUENCED:
			col_append_str(pinfo->cinfo, COL_INFO, "Send unsequenced, ");
			break;

		case ENET_PROTOCOL_COMMAND_SEND_FRAGMENT:
			pinfo->fragmented = TRUE;
			offset = dissect_enet_sendFragment(tvb, pinfo, enetTree, command, offset);
			col_append_str(pinfo->cinfo, COL_INFO, "Send fragment, ");
			break;

		case ENET_PROTOCOL_COMMAND_BANDWIDTH_LIMIT:
			col_append_str(pinfo->cinfo, COL_INFO, "Bandwidth limit, ");
			break;

		case ENET_PROTOCOL_COMMAND_THROTTLE_CONFIGURE:
			col_append_str(pinfo->cinfo, COL_INFO, "Throttle configure, ");
			break;
		default:
			col_append_str(pinfo->cinfo, COL_INFO, "Unknown, ");
	}
	return offset;
}

static void dissect_lol(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree)
{
	//Enet
	size_t headerSize;
	unsigned char *enetData = NULL;
	ENetProtocolHeader * header;
	enet_uint16 peerID, flags;
	guint offset = 0;

	/* Clear out stuff in the info column */
	col_set_str(pinfo->cinfo, COL_PROTOCOL, "LoL");
	col_clear(pinfo->cinfo, COL_INFO);
	
	if(tree)
	{
		proto_item *enetNode = NULL;
		proto_item *enetHeader = NULL, *item = NULL;
		proto_tree *enetTree = NULL;
		

		enetData = (unsigned char*)tvb_get_ptr(tvb, 0, tvb_length(tvb));

		header = (ENetProtocolHeader *)enetData;
		peerID = ENET_NET_TO_HOST_16 (header -> peerID);
		flags = peerID & ENET_PROTOCOL_HEADER_FLAG_MASK;
		peerID &= ~ ENET_PROTOCOL_HEADER_FLAG_MASK;
		headerSize = (flags & ENET_PROTOCOL_HEADER_FLAG_SENT_TIME ? sizeof (ENetProtocolHeader) : (size_t) & ((ENetProtocolHeader *) 0) -> sentTime);

		/* Top level header */
		enetNode = proto_tree_add_item(tree, proto_enet, tvb, 0, -1, FALSE);
		enetTree = proto_item_add_subtree(enetNode, ett_enet);
		item = proto_tree_add_item(enetTree, hf_enet_header, tvb, 0, headerSize, FALSE);
		enetHeader = proto_item_add_subtree(item, ett_enet);

		/* Show all the extracted info in the dissector for enet header */
		proto_tree_add_item(enetHeader, hf_enet_checksum, tvb, offset, 4, ENC_BIG_ENDIAN); offset += 4;
		proto_tree_add_uint(enetHeader, hf_enet_peerId, tvb, offset, 2, peerID);
		proto_tree_add_uint(enetHeader, hf_enet_flags, tvb, offset, 2, flags); offset += 2;
		if(flags & ENET_PROTOCOL_HEADER_FLAG_SENT_TIME)
		{
			proto_tree_add_item(enetHeader, hf_enet_sentTime, tvb, offset, 2, ENC_BIG_ENDIAN); offset += 2;
		}

		offset = dissect_command(tvb, pinfo, enetTree, enetData, offset);
	}
}

tvbuff_t *decrypt_lol(tvbuff_t *tvb, guint16 data_length)
{
	return NULL;
}