#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemastypes.h>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

typedef struct{
    char ticket_id[20];
    char timestamp[50];
    float baggage_weight;
    int loyalty_points;
    char status[10];
    char destination[40];
    char cabin_class[10];
    int seat_num;
    char app_ver[10];
    char passanger_name[60];
} RECORD;

void root_name(const char *filepath, char *out_name) {
    const char *base = filepath;
    const char *last_slash = strrchr(filepath, '/');
    if (last_slash) base = last_slash + 1;
    last_slash = strrchr(base, '\\');
    if (last_slash) base = last_slash + 1;

    strcpy(out_name, base);
    char *dot = strrchr(out_name, '.');
    if (dot) *dot = '\0';
}

void clean_crlf(char *str) {
    int len = strlen(str);
    while (len > 0 && (str[len-1] == '\r' || str[len-1] == '\n')) {
        str[len-1] = '\0';
        len--;
    }
}

void csv_to_binary(const char *input_file, const char *output_file, int sep_type, int opsys) {
    FILE *pFile = fopen(input_file, "r");
    FILE *pBinaryFile = fopen(output_file, "wb");
    char buffer[1024];

    if (pFile == NULL || pBinaryFile == NULL) {
        printf("Error opening file\n");
        if (pFile) fclose(pFile);
        if (pBinaryFile) fclose(pBinaryFile);
        return;
    }

    char sep = ',';
    if (sep_type == 2) sep = '\t';
    else if (sep_type == 3) sep = ';';

    // Header skip
    if (fgets(buffer, sizeof(buffer), pFile) == NULL) {
        fclose(pFile);
        fclose(pBinaryFile);
        return;
    }

    while (fgets(buffer, sizeof(buffer), pFile) != NULL) {
        clean_crlf(buffer);
        if (strlen(buffer) == 0) continue;

        RECORD rec;
        memset(&rec, 0, sizeof(RECORD));

        char *ptr = buffer;
        char *token;
        int col = 0;
        
        while (ptr != NULL && col < 10) {
            token = ptr;
            char *next = strchr(ptr, sep);
            if (next) {
                *next = '\0';
                ptr = next + 1;
            } else {
                ptr = NULL;
            }

            switch(col) {
                case 0: strncpy(rec.ticket_id, token, 19); break;
                case 1: strncpy(rec.timestamp, token, 49); break;
                case 2: rec.baggage_weight = (token[0] != '\0') ? atof(token) : 0.0f; break;
                case 3: rec.loyalty_points = (token[0] != '\0') ? atoi(token) : 0; break;
                case 4: strncpy(rec.status, token, 9); break;
                case 5: strncpy(rec.destination, token, 39); break;
                case 6: strncpy(rec.cabin_class, token, 9); break;
                case 7: rec.seat_num = (token[0] != '\0') ? atoi(token) : 0; break;
                case 8: strncpy(rec.app_ver, token, 9); break;
                case 9: strncpy(rec.passanger_name, token, 59); break;
            }
            col++;
        }
        
        fwrite(&rec, sizeof(RECORD), 1, pBinaryFile);
    }
    fclose(pFile);
    fclose(pBinaryFile);
}

void binary_to_xml(const char *input_file, const char *output_file) {
    char root_element_name[256] = {0};
    root_name(output_file, root_element_name);

    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "flightlogs");
    xmlDocSetRootElement(doc, root_node);
    
    RECORD flight_records;
    int entry = 1;

    FILE *pbinaryFile = fopen(input_file, "rb");
    if (pbinaryFile == NULL) {
        printf("Error opening file\n");
        return;
    }

    while (fread(&flight_records, sizeof(RECORD), 1, pbinaryFile) == 1) {
        char baggage_weight[50], seat_num[50], loyalty_points[50], id[10];
        
        // Use proper dot formatting for float
        sprintf(baggage_weight, "%.1f", flight_records.baggage_weight);
        sprintf(seat_num, "%d", flight_records.seat_num);
        sprintf(loyalty_points, "%d", flight_records.loyalty_points);
        sprintf(id, "%d", entry);

        xmlNodePtr entry_node = xmlNewChild(root_node, NULL, BAD_CAST "entry", NULL);
        xmlNewProp(entry_node, BAD_CAST "id", BAD_CAST id);

        xmlNodePtr ticket_node = xmlNewChild(entry_node, NULL, BAD_CAST "ticket", NULL);
        xmlNewTextChild(ticket_node, NULL, BAD_CAST "ticket_id", BAD_CAST flight_records.ticket_id);
        if (strlen(flight_records.destination) > 0)
            xmlNewTextChild(ticket_node, NULL, BAD_CAST "destination", BAD_CAST flight_records.destination);
        if (strlen(flight_records.app_ver) > 0)
            xmlNewTextChild(ticket_node, NULL, BAD_CAST "app_ver", BAD_CAST flight_records.app_ver);

        xmlNodePtr metrics_node = xmlNewChild(entry_node, NULL, BAD_CAST "metrics", NULL);
        xmlNewProp(metrics_node, BAD_CAST "status", BAD_CAST flight_records.status);
        xmlNewProp(metrics_node, BAD_CAST "cabin_class", BAD_CAST flight_records.cabin_class);

        xmlNewTextChild(metrics_node, NULL, BAD_CAST "baggage_weight", BAD_CAST baggage_weight);
        if (flight_records.loyalty_points > 0)
            xmlNewTextChild(metrics_node, NULL, BAD_CAST "loyalty_points", BAD_CAST loyalty_points);
        xmlNewTextChild(metrics_node, NULL, BAD_CAST "seat_num", BAD_CAST seat_num);

        xmlNewTextChild(entry_node, NULL, BAD_CAST "timestamp", BAD_CAST flight_records.timestamp);

        unsigned char *p_name = (unsigned char *)flight_records.passanger_name;
        char hex_str[15] = {0};

        if ((p_name[0] & 0x80) == 0) {
            sprintf(hex_str, "%02X", p_name[0]);
        } else if ((p_name[0] & 0xE0) == 0xC0) {
            sprintf(hex_str, "%02X%02X", p_name[0], p_name[1]);
        } else if ((p_name[0] & 0xF0) == 0xE0) {
            sprintf(hex_str, "%02X%02X%02X", p_name[0], p_name[1], p_name[2]);
        } else if ((p_name[0] & 0xF8) == 0xF0) {
            sprintf(hex_str, "%02X%02X%02X%02X", p_name[0], p_name[1], p_name[2], p_name[3]);
        }

        xmlNodePtr pass_node = xmlNewChild(entry_node, NULL, BAD_CAST "passenger_name", BAD_CAST flight_records.passanger_name);
        xmlNewProp(pass_node, BAD_CAST "current_encoding", BAD_CAST "UTF-8");
        xmlNewProp(pass_node, BAD_CAST "first_char_hex", BAD_CAST hex_str);
        
        entry++;
    }
    fclose(pbinaryFile);
    xmlSaveFormatFileEnc(output_file, doc, "UTF-8", 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();
}

void validate(const char *xmlfile , const char *xsdfile) {
    xmlDocPtr doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;

    xmlLineNumbersDefault(1);
    ctxt = xmlSchemaNewParserCtxt(xsdfile);
    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);

    doc = xmlReadFile(xmlfile, NULL, 0);
    if (doc == NULL)
    {
        fprintf(stderr, "Could not parse %s\n", xmlfile);
    }
    else
    {
        xmlSchemaValidCtxtPtr ctxt2;
        int ret;

        ctxt2 = xmlSchemaNewValidCtxt(schema);
        ret = xmlSchemaValidateDoc(ctxt2, doc);
        if (ret == 0)
        {
            printf("%s validates\n", xmlfile);
        }
        else if (ret > 0)
        {
            printf("%s fails to validate\n", xmlfile);
        }
        else
        {
            printf("%s validation generated an internal error\n", xmlfile);
        }
        xmlSchemaFreeValidCtxt(ctxt2);
        xmlFreeDoc(doc);
    }
    if(schema != NULL)
        xmlSchemaFree(schema);

    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();
}

// Convert endianness and string values for type 4
void xml_encoding(const char *input, const char *output, int enc_type) {
    xmlDocPtr doc = xmlReadFile(input, NULL, XML_PARSE_NOBLANKS);
    if (doc == NULL) {
        printf("Error: Could not parse XML file %s\n", input);
        return;
    }
    
    const char *encoding_str = "UTF-8";
    if (enc_type == 1) encoding_str = "UTF-16LE";
    else if (enc_type == 2) encoding_str = "UTF-16BE";
    
    xmlNodePtr root = xmlDocGetRootElement(doc);
    xmlNodePtr entry = root->children;
    
    while (entry != NULL) {
        if (entry->type == XML_ELEMENT_NODE && xmlStrcmp(entry->name, BAD_CAST "entry") == 0) {
            xmlNodePtr child = entry->children;
            while (child != NULL) {
                if (child->type == XML_ELEMENT_NODE && xmlStrcmp(child->name, BAD_CAST "passenger_name") == 0) {
                    // Update current_encoding attribute
                    xmlSetProp(child, BAD_CAST "current_encoding", BAD_CAST encoding_str);
                    
                    // We need to re-evaluate the first char length to update the first_char_hex
                    // If going to UTF-16, logic depends on LE/BE and character code point.
                    // Actually, UTF-8 char -> decoding it -> encode in UTF-16 BE or LE -> output as hex
                    xmlChar *content = xmlNodeGetContent(child);
                    if (content != NULL && strlen((char*)content) > 0) {
                        unsigned char *p = (unsigned char *)content;
                        unsigned int codepoint = 0;
                        if ((p[0] & 0x80) == 0) codepoint = p[0];
                        else if ((p[0] & 0xE0) == 0xC0) codepoint = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
                        else if ((p[0] & 0xF0) == 0xE0) codepoint = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
                        else if ((p[0] & 0xF8) == 0xF0) codepoint = ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
                        
                        char hex_str[20] = {0};
                        if (enc_type == 1) {
                            // UTF-16LE
                            if (codepoint < 0x10000) {
                                sprintf(hex_str, "%02X%02X", codepoint & 0xFF, (codepoint >> 8) & 0xFF);
                            } else {
                                codepoint -= 0x10000;
                                unsigned int high = 0xD800 + (codepoint >> 10);
                                unsigned int low = 0xDC00 + (codepoint & 0x3FF);
                                sprintf(hex_str, "%02X%02X%02X%02X", high & 0xFF, (high >> 8) & 0xFF, low & 0xFF, (low >> 8) & 0xFF);
                            }
                        } else if (enc_type == 2) {
                            // UTF-16BE
                            if (codepoint < 0x10000) {
                                sprintf(hex_str, "%02X%02X", (codepoint >> 8) & 0xFF, codepoint & 0xFF);
                            } else {
                                codepoint -= 0x10000;
                                unsigned int high = 0xD800 + (codepoint >> 10);
                                unsigned int low = 0xDC00 + (codepoint & 0x3FF);
                                sprintf(hex_str, "%02X%02X%02X%02X", (high >> 8) & 0xFF, high & 0xFF, (low >> 8) & 0xFF, low & 0xFF);
                            }
                        } else {
                            // UTF-8
                            if ((p[0] & 0x80) == 0) sprintf(hex_str, "%02X", p[0]);
                            else if ((p[0] & 0xE0) == 0xC0) sprintf(hex_str, "%02X%02X", p[0], p[1]);
                            else if ((p[0] & 0xF0) == 0xE0) sprintf(hex_str, "%02X%02X%02X", p[0], p[1], p[2]);
                            else if ((p[0] & 0xF8) == 0xF0) sprintf(hex_str, "%02X%02X%02X%02X", p[0], p[1], p[2], p[3]);
                        }
                        xmlSetProp(child, BAD_CAST "first_char_hex", BAD_CAST hex_str);
                        xmlFree(content);
                    }
                }
                child = child->next;
            }
        }
        entry = entry->next;
    }
    
    xmlCharEncodingHandlerPtr handler = NULL;
    if (enc_type == 1) handler = xmlFindCharEncodingHandler("UTF-16LE");
    else if (enc_type == 2) handler = xmlFindCharEncodingHandler("UTF-16BE");
    else handler = xmlFindCharEncodingHandler("UTF-8");

    xmlSaveFormatFileEnc(output, doc, encoding_str, 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
    // Fix BOM if libxml didn't write it correctly or write it specifically
    if (enc_type == 1 || enc_type == 2) {
        FILE *f = fopen(output, "rb");
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        unsigned char *buf = malloc(fsize);
        fread(buf, 1, fsize, f);
        fclose(f);
        
        // Remove existing BOM if any
        long offset = 0;
        if (buf[0] == 0xFF && buf[1] == 0xFE) offset = 2;
        if (buf[0] == 0xFE && buf[1] == 0xFF) offset = 2;
        
        FILE *fw = fopen(output, "wb");
        if (enc_type == 1) {
            fputc(0xFF, fw); fputc(0xFE, fw);
        } else {
            fputc(0xFE, fw); fputc(0xFF, fw);
        }
        fwrite(buf + offset, 1, fsize - offset, fw);
        fclose(fw);
        free(buf);
    }
}

void print_help() {
    printf("Usage:\n");
    printf("./flightTool <input_file> <output_file> <conversion_type> -separator <1|2|3> -opsys <1|2|3> [-encoding <1|2|3>] [-h]\n\n");
    printf("Arguments:\n");
    printf("<input_file>        = Source file to read from\n");
    printf("<output_file>       = Target file to write to\n");
    printf("<conversion_type>   = 1 -> CSV to Binary\n");
    printf("                      2 -> Binary to XML\n");
    printf("                      3 -> XML validation with XSD\n");
    printf("                      4 -> XML Encoding Converter (XML to XML)\n");
    printf("-separator <P1>     = 1=comma, 2=tab, 3=semicolon\n");
    printf("-opsys <P2>         = 1=windows, 2=linux, 3=macos\n");
    printf("-encoding <P3>      = 1=UTF-16LE, 2=UTF-16BE, 3=UTF-8\n");
}


int main(int argc, char **argv) {
    if (argc < 2) {
        print_help();
        return 1;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_help();
        return 0;
    }

    if (argc < 4) {
        printf("Error: Missing arguments\n");
        print_help();
        return 1;
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];
    int conversion_type = atoi(argv[3]);

    int sep_type = 1;
    int opsys = 2;
    int encoding = 0;

    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "-separator") == 0 && i + 1 < argc) {
            sep_type = atoi(argv[i+1]);
            i++;
        } else if (strcmp(argv[i], "-opsys") == 0 && i + 1 < argc) {
            opsys = atoi(argv[i+1]);
            i++;
        } else if (strcmp(argv[i], "-encoding") == 0 && i + 1 < argc) {
            encoding = atoi(argv[i+1]);
            i++;
        } else if (strcmp(argv[i], "-h") == 0) {
            print_help();
            return 0;
        }
    }

    if (conversion_type == 1) {
        csv_to_binary(input_file, output_file, sep_type, opsys);
        printf("CSV to Binary conversion done.\n");
    } else if (conversion_type == 2) {
        binary_to_xml(input_file, output_file);
        printf("Binary to XML conversion done.\n");
    } else if (conversion_type == 3) {
        validate(input_file, output_file);
    } else if (conversion_type == 4) {
        if (encoding == 0) {
            printf("Error: -encoding flag is required for conversion type 4.\n");
            return 1;
        }
        xml_encoding(input_file, output_file, encoding);
        printf("XML encoding conversion done.\n");
    } else {
        printf("Error: Invalid conversion type %d\n", conversion_type);
        return 1;
    }

    return 0;
}
