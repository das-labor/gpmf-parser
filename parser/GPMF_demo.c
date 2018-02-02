/*! @file GPMF_demo.c
 *
 *  @brief Demo to extract GPMF from an MP4
 *
 *  @version 1.0.1
 *
 *  (C) Copyright 2017 GoPro Inc (http://gopro.com/).
 *	
 *  Licensed under either:
 *  - Apache License, Version 2.0, http://www.apache.org/licenses/LICENSE-2.0  
 *  - MIT license, http://opensource.org/licenses/MIT
 *  at your option.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../GPMF_parser.h"
#include "GPMF_mp4reader.h"


extern void PrintGPMF(GPMF_stream *ms);

int main(int argc, char *argv[])
{
	int32_t ret = GPMF_OK;
	GPMF_stream metadata_stream, *ms = &metadata_stream;
	double metadatalength;
	uint32_t *payload = NULL; //buffer to store GPMF samples from the MP4.


	// get file return data
	if (argc != 2)
	{
		printf("usage: %s <file_with_GPMF>\n", argv[0]);
		return -1;
	}


	metadatalength = OpenGPMFSource(argv[1]);

	//metadatalength = OpenGPMFSourceUDTA(argv[1]);

	if (metadatalength > 0.0)
	{
		uint32_t index, payloads = GetNumberGPMFPayloads();
//		printf("found %.2fs of metadata, from %d payloads, within %s\n", metadatalength, payloads, argv[1]);

  printf("[");

#if 1
		if (payloads) // Printf the contents of the single payload
		{
			uint32_t payloadsize = GetGPMFPayloadSize(0);
			payload = GetGPMFPayload(payload, 0);
			if(payload == NULL)
				goto cleanup;

			ret = GPMF_Init(ms, payload, payloadsize);
			if (ret != GPMF_OK)
				goto cleanup;

			// Output (printf) all the contained GPMF data within this payload
			ret = GPMF_Validate(ms, GPMF_RECURSE_LEVELS); // optional
			if (GPMF_OK != ret)
			{
				printf("Invalid Structure\n");
				goto cleanup;
			}

			GPMF_ResetState(ms);
			do
			{
				//PrintGPMF(ms);  // printf current GPMF KLV
			} while (GPMF_OK == GPMF_Next(ms, GPMF_RECURSE_LEVELS));
			GPMF_ResetState(ms);
			printf("\n");

		}
#endif


		for (index = 0; index < payloads; index++)
		{
			uint32_t payloadsize = GetGPMFPayloadSize(index);
			double in = 0.0, out = 0.0; //times
			payload = GetGPMFPayload(payload, index);
			if (payload == NULL)
				goto cleanup;

			ret = GetGPMFPayloadTime(index, &in, &out);
			if (ret != GPMF_OK)
				goto cleanup;

			ret = GPMF_Init(ms, payload, payloadsize);
			if (ret != GPMF_OK)
				goto cleanup;


#if 1		// Find GPS values and return scaled doubles. 
			if (index) // show first payload would be (index == 1)
			{
				if (GPMF_OK == GPMF_FindNext(ms, STR2FOURCC("GPS5"), GPMF_RECURSE_LEVELS)) //|| //GoPro Hero5 GPS
				{
					uint32_t key = GPMF_Key(ms);
					uint32_t samples = GPMF_Repeat(ms);
					uint32_t elements = GPMF_ElementsInStruct(ms);
  				uint32_t buffersize = samples * elements * sizeof(double);
					GPMF_stream find_stream;
					double *ptr, *tmpbuffer = malloc(buffersize);
					char units[10][6] = { "" };
					uint32_t unit_samples = 1;

          printf("{\n");
					printf("\"starttime\":\"%f\",\n\"endtime\":\"%f\",\n", in, out);

					if (tmpbuffer && samples)
					{
						uint32_t i, j;

						//Search for any units to display
						GPMF_CopyState(ms, &find_stream);
						if (GPMF_OK == GPMF_FindPrev(&find_stream, GPMF_KEY_SI_UNITS, GPMF_CURRENT_LEVEL) ||
							GPMF_OK == GPMF_FindPrev(&find_stream, GPMF_KEY_UNITS, GPMF_CURRENT_LEVEL))
						{
							char *data = (char *)GPMF_RawData(&find_stream);
							int ssize = GPMF_StructSize(&find_stream);
							unit_samples = GPMF_Repeat(&find_stream);

							for (i = 0; i < unit_samples; i++)
							{
								memcpy(units[i], data, ssize);
								units[i][ssize] = 0;
								data += ssize;
							}
						}

						GPMF_ScaledData(ms, tmpbuffer, buffersize, 0, samples, GPMF_TYPE_DOUBLE);  //Output scaled data as floats

						ptr = tmpbuffer;
//            printf("\"geo\": [\n");
            
//						for (i = 0; i < samples; i++)
//						{
//            for (j = 0; j < elements; j++)
//              for (j = 0; j = 0; j++)
//								printf("{\"lat\":\"%.3f\", \"lon\":\"%.3f\"},\n", ptr[0], ptr[1]); // unit = 'units[j%unit_samples]'
//            }
//            printf("],");
            if (index == (payloads-1))
            {
              printf("\"lat\":\"%f\",\n\"lon\":\"%f\"\n", ptr[0], ptr[1]);
              printf("}\n");
						  free(tmpbuffer);
            } else {
              printf("\"lat\":\"%f\",\n\"lon\":\"%f\"\n", ptr[0], ptr[1]);
              printf("},\n");
              free(tmpbuffer);
            }
					}
				}
				GPMF_ResetState(ms);
			}
#endif 
    }
    printf("]\n");

	cleanup:
		if (payload) FreeGPMFPayload(payload); payload = NULL;
		CloseGPMFSource();
	}

	return ret;
}
