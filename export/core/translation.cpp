/*************************************************************************/
/*  translation.cpp                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "translation.h"

#include "core/io/resource_loader.h"
#include "core/os/os.h"
#include "core/project_settings.h"

// ISO 639-1 language codes (and a couple of three-letter ISO 639-2 codes),
// with the addition of glibc locales with their regional identifiers.
// This list must match the language names (in English) of locale_names.
//
// References:
// - https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
// - https://lh.2xlibre.net/locales/
// - https://iso639-3.sil.org/

static const char *locale_list[] = {
	"aa", //  Afar
	"aa_DJ", //  Afar (Djibouti)
	"aa_ER", //  Afar (Eritrea)
	"aa_ET", //  Afar (Ethiopia)
	"af", //  Afrikaans
	"af_ZA", //  Afrikaans (South Africa)
	"agr_PE", //  Aguaruna (Peru)
	"ak_GH", //  Akan (Ghana)
	"am_ET", //  Amharic (Ethiopia)
	"an_ES", //  Aragonese (Spain)
	"anp_IN", //  Angika (India)
	"ar", //  Arabic
	"ar_AE", //  Arabic (United Arab Emirates)
	"ar_BH", //  Arabic (Bahrain)
	"ar_DZ", //  Arabic (Algeria)
	"ar_EG", //  Arabic (Egypt)
	"ar_IN", //  Arabic (India)
	"ar_IQ", //  Arabic (Iraq)
	"ar_JO", //  Arabic (Jordan)
	"ar_KW", //  Arabic (Kuwait)
	"ar_LB", //  Arabic (Lebanon)
	"ar_LY", //  Arabic (Libya)
	"ar_MA", //  Arabic (Morocco)
	"ar_OM", //  Arabic (Oman)
	"ar_QA", //  Arabic (Qatar)
	"ar_SA", //  Arabic (Saudi Arabia)
	"ar_SD", //  Arabic (Sudan)
	"ar_SS", //  Arabic (South Soudan)
	"ar_SY", //  Arabic (Syria)
	"ar_TN", //  Arabic (Tunisia)
	"ar_YE", //  Arabic (Yemen)
	"as_IN", //  Assamese (India)
	"ast_ES", //  Asturian (Spain)
	"ayc_PE", //  Southern Aymara (Peru)
	"ay_PE", //  Aymara (Peru)
	"az", //  Azerbaijani
	"az_AZ", //  Azerbaijani (Azerbaijan)
	"be", //  Belarusian
	"be_BY", //  Belarusian (Belarus)
	"bem_ZM", //  Bemba (Zambia)
	"ber_DZ", //  Berber languages (Algeria)
	"ber_MA", //  Berber languages (Morocco)
	"bg", //  Bulgarian
	"bg_BG", //  Bulgarian (Bulgaria)
	"bhb_IN", //  Bhili (India)
	"bho_IN", //  Bhojpuri (India)
	"bi_TV", //  Bislama (Tuvalu)
	"bn", //  Bengali
	"bn_BD", //  Bengali (Bangladesh)
	"bn_IN", //  Bengali (India)
	"bo", //  Tibetan
	"bo_CN", //  Tibetan (China)
	"bo_IN", //  Tibetan (India)
	"br", //  Breton
	"br_FR", //  Breton (France)
	"brx_IN", //  Bodo (India)
	"bs_BA", //  Bosnian (Bosnia and Herzegovina)
	"byn_ER", //  Bilin (Eritrea)
	"ca", //  Catalan
	"ca_AD", //  Catalan (Andorra)
	"ca_ES", //  Catalan (Spain)
	"ca_FR", //  Catalan (France)
	"ca_IT", //  Catalan (Italy)
	"ce_RU", //  Chechen (Russia)
	"chr_US", //  Cherokee (United States)
	"cmn_TW", //  Mandarin Chinese (Taiwan)
	"crh_UA", //  Crimean Tatar (Ukraine)
	"csb_PL", //  Kashubian (Poland)
	"cs", //  Czech
	"cs_CZ", //  Czech (Czech Republic)
	"cv_RU", //  Chuvash (Russia)
	"cy_GB", //  Welsh (United Kingdom)
	"da", //  Danish
	"da_DK", //  Danish (Denmark)
	"de", //  German
	"de_AT", //  German (Austria)
	"de_BE", //  German (Belgium)
	"de_CH", //  German (Switzerland)
	"de_DE", //  German (Germany)
	"de_IT", //  German (Italy)
	"de_LU", //  German (Luxembourg)
	"doi_IN", //  Dogri (India)
	"dv_MV", //  Dhivehi (Maldives)
	"dz_BT", //  Dzongkha (Bhutan)
	"el", //  Greek
	"el_CY", //  Greek (Cyprus)
	"el_GR", //  Greek (Greece)
	"en", //  English
	"en_AG", //  English (Antigua and Barbuda)
	"en_AU", //  English (Australia)
	"en_BW", //  English (Botswana)
	"en_CA", //  English (Canada)
	"en_DK", //  English (Denmark)
	"en_GB", //  English (United Kingdom)
	"en_HK", //  English (Hong Kong)
	"en_IE", //  English (Ireland)
	"en_IL", //  English (Israel)
	"en_IN", //  English (India)
	"en_NG", //  English (Nigeria)
	"en_NZ", //  English (New Zealand)
	"en_PH", //  English (Philippines)
	"en_SG", //  English (Singapore)
	"en_US", //  English (United States)
	"en_ZA", //  English (South Africa)
	"en_ZM", //  English (Zambia)
	"en_ZW", //  English (Zimbabwe)
	"eo", //  Esperanto
	"es", //  Spanish
	"es_AR", //  Spanish (Argentina)
	"es_BO", //  Spanish (Bolivia)
	"es_CL", //  Spanish (Chile)
	"es_CO", //  Spanish (Colombia)
	"es_CR", //  Spanish (Costa Rica)
	"es_CU", //  Spanish (Cuba)
	"es_DO", //  Spanish (Dominican Republic)
	"es_EC", //  Spanish (Ecuador)
	"es_ES", //  Spanish (Spain)
	"es_GT", //  Spanish (Guatemala)
	"es_HN", //  Spanish (Honduras)
	"es_MX", //  Spanish (Mexico)
	"es_NI", //  Spanish (Nicaragua)
	"es_PA", //  Spanish (Panama)
	"es_PE", //  Spanish (Peru)
	"es_PR", //  Spanish (Puerto Rico)
	"es_PY", //  Spanish (Paraguay)
	"es_SV", //  Spanish (El Salvador)
	"es_US", //  Spanish (United States)
	"es_UY", //  Spanish (Uruguay)
	"es_VE", //  Spanish (Venezuela)
	"et", //  Estonian
	"et_EE", //  Estonian (Estonia)
	"eu", //  Basque
	"eu_ES", //  Basque (Spain)
	"fa", //  Persian
	"fa_IR", //  Persian (Iran)
	"ff_SN", //  Fulah (Senegal)
	"fi", //  Finnish
	"fi_FI", //  Finnish (Finland)
	"fil", //  Filipino
	"fil_PH", //  Filipino (Philippines)
	"fo_FO", //  Faroese (Faroe Islands)
	"fr", //  French
	"fr_BE", //  French (Belgium)
	"fr_CA", //  French (Canada)
	"fr_CH", //  French (Switzerland)
	"fr_FR", //  French (France)
	"fr_LU", //  French (Luxembourg)
	"fur_IT", //  Friulian (Italy)
	"fy_DE", //  Western Frisian (Germany)
	"fy_NL", //  Western Frisian (Netherlands)
	"ga", //  Irish
	"ga_IE", //  Irish (Ireland)
	"gd_GB", //  Scottish Gaelic (United Kingdom)
	"gez_ER", //  Geez (Eritrea)
	"gez_ET", //  Geez (Ethiopia)
	"gl", //  Galician
	"gl_ES", //  Galician (Spain)
	"gu_IN", //  Gujarati (India)
	"gv_GB", //  Manx (United Kingdom)
	"hak_TW", //  Hakka Chinese (Taiwan)
	"ha_NG", //  Hausa (Nigeria)
	"he", //  Hebrew
	"he_IL", //  Hebrew (Israel)
	"hi", //  Hindi
	"hi_IN", //  Hindi (India)
	"hne_IN", //  Chhattisgarhi (India)
	"hr", //  Croatian
	"hr_HR", //  Croatian (Croatia)
	"hsb_DE", //  Upper Sorbian (Germany)
	"ht_HT", //  Haitian (Haiti)
	"hu", //  Hungarian
	"hu_HU", //  Hungarian (Hungary)
	"hus_MX", //  Huastec (Mexico)
	"hy_AM", //  Armenian (Armenia)
	"ia_FR", //  Interlingua (France)
	"id", //  Indonesian
	"id_ID", //  Indonesian (Indonesia)
	"ig_NG", //  Igbo (Nigeria)
	"ik_CA", //  Inupiaq (Canada)
	"is", //  Icelandic
	"is_IS", //  Icelandic (Iceland)
	"it", //  Italian
	"it_CH", //  Italian (Switzerland)
	"it_IT", //  Italian (Italy)
	"iu_CA", //  Inuktitut (Canada)
	"ja", //  Japanese
	"ja_JP", //  Japanese (Japan)
	"kab_DZ", //  Kabyle (Algeria)
	"ka", //  Georgian
	"ka_GE", //  Georgian (Georgia)
	"kk_KZ", //  Kazakh (Kazakhstan)
	"kl_GL", //  Kalaallisut (Greenland)
	"km", //  Central Khmer
	"km_KH", //  Central Khmer (Cambodia)
	"kn_IN", //  Kannada (India)
	"kok_IN", //  Konkani (India)
	"ko", //  Korean
	"ko_KR", //  Korean (South Korea)
	"ks_IN", //  Kashmiri (India)
	"ku", //  Kurdish
	"ku_TR", //  Kurdish (Turkey)
	"kw_GB", //  Cornish (United Kingdom)
	"ky_KG", //  Kirghiz (Kyrgyzstan)
	"lb_LU", //  Luxembourgish (Luxembourg)
	"lg_UG", //  Ganda (Uganda)
	"li_BE", //  Limburgan (Belgium)
	"li_NL", //  Limburgan (Netherlands)
	"lij_IT", //  Ligurian (Italy)
	"ln_CD", //  Lingala (Congo)
	"lo_LA", //  Lao (Laos)
	"lt", //  Lithuanian
	"lt_LT", //  Lithuanian (Lithuania)
	"lv", //  Latvian
	"lv_LV", //  Latvian (Latvia)
	"lzh_TW", //  Literary Chinese (Taiwan)
	"mag_IN", //  Magahi (India)
	"mai_IN", //  Maithili (India)
	"mg_MG", //  Malagasy (Madagascar)
	"mh_MH", //  Marshallese (Marshall Islands)
	"mhr_RU", //  Eastern Mari (Russia)
	"mi", //  Māori
	"mi_NZ", //  Māori (New Zealand)
	"miq_NI", //  Mískito (Nicaragua)
	"mk", //  Macedonian
	"mk_MK", //  Macedonian (Macedonia)
	"ml", //  Malayalam
	"ml_IN", //  Malayalam (India)
	"mni_IN", //  Manipuri (India)
	"mn_MN", //  Mongolian (Mongolia)
	"mr", //  Marathi
	"mr_IN", //  Marathi (India)
	"ms", //  Malay
	"ms_MY", //  Malay (Malaysia)
	"mt", //  Maltese
	"mt_MT", //  Maltese (Malta)
	"my_MM", //  Burmese (Myanmar)
	"myv_RU", //  Erzya (Russia)
	"nah_MX", //  Nahuatl languages (Mexico)
	"nan_TW", //  Min Nan Chinese (Taiwan)
	"nb", //  Norwegian Bokmål
	"nb_NO", //  Norwegian Bokmål (Norway)
	"nds_DE", //  Low German (Germany)
	"nds_NL", //  Low German (Netherlands)
	"ne_NP", //  Nepali (Nepal)
	"nhn_MX", //  Central Nahuatl (Mexico)
	"niu_NU", //  Niuean (Niue)
	"niu_NZ", //  Niuean (New Zealand)
	"nl", //  Dutch
	"nl_AW", //  Dutch (Aruba)
	"nl_BE", //  Dutch (Belgium)
	"nl_NL", //  Dutch (Netherlands)
	"nn", //  Norwegian Nynorsk
	"nn_NO", //  Norwegian Nynorsk (Norway)
	"nr_ZA", //  South Ndebele (South Africa)
	"nso_ZA", //  Pedi (South Africa)
	"oc_FR", //  Occitan (France)
	"om", //  Oromo
	"om_ET", //  Oromo (Ethiopia)
	"om_KE", //  Oromo (Kenya)
	"or", //  Oriya
	"or_IN", //  Oriya (India)
	"os_RU", //  Ossetian (Russia)
	"pa_IN", //  Panjabi (India)
	"pap", //  Papiamento
	"pap_AN", //  Papiamento (Netherlands Antilles)
	"pap_AW", //  Papiamento (Aruba)
	"pap_CW", //  Papiamento (Curaçao)
	"pa_PK", //  Panjabi (Pakistan)
	"pl", //  Polish
	"pl_PL", //  Polish (Poland)
	"pr", //  Pirate
	"ps_AF", //  Pushto (Afghanistan)
	"pt", //  Portuguese
	"pt_BR", //  Portuguese (Brazil)
	"pt_PT", //  Portuguese (Portugal)
	"quy_PE", //  Ayacucho Quechua (Peru)
	"quz_PE", //  Cusco Quechua (Peru)
	"raj_IN", //  Rajasthani (India)
	"ro", //  Romanian
	"ro_RO", //  Romanian (Romania)
	"ru", //  Russian
	"ru_RU", //  Russian (Russia)
	"ru_UA", //  Russian (Ukraine)
	"rw_RW", //  Kinyarwanda (Rwanda)
	"sa_IN", //  Sanskrit (India)
	"sat_IN", //  Santali (India)
	"sc_IT", //  Sardinian (Italy)
	"sco", //  Scots
	"sd_IN", //  Sindhi (India)
	"se_NO", //  Northern Sami (Norway)
	"sgs_LT", //  Samogitian (Lithuania)
	"shs_CA", //  Shuswap (Canada)
	"sid_ET", //  Sidamo (Ethiopia)
	"si", //  Sinhala
	"si_LK", //  Sinhala (Sri Lanka)
	"sk", //  Slovak
	"sk_SK", //  Slovak (Slovakia)
	"sl", //  Slovenian
	"sl_SI", //  Slovenian (Slovenia)
	"so", //  Somali
	"so_DJ", //  Somali (Djibouti)
	"so_ET", //  Somali (Ethiopia)
	"so_KE", //  Somali (Kenya)
	"so_SO", //  Somali (Somalia)
	"son_ML", //  Songhai languages (Mali)
	"sq", //  Albanian
	"sq_AL", //  Albanian (Albania)
	"sq_KV", //  Albanian (Kosovo)
	"sq_MK", //  Albanian (Macedonia)
	"sr", //  Serbian
	"sr_Cyrl", //  Serbian (Cyrillic)
	"sr_Latn", //  Serbian (Latin)
	"sr_ME", //  Serbian (Montenegro)
	"sr_RS", //  Serbian (Serbia)
	"ss_ZA", //  Swati (South Africa)
	"st_ZA", //  Southern Sotho (South Africa)
	"sv", //  Swedish
	"sv_FI", //  Swedish (Finland)
	"sv_SE", //  Swedish (Sweden)
	"sw_KE", //  Swahili (Kenya)
	"sw_TZ", //  Swahili (Tanzania)
	"szl_PL", //  Silesian (Poland)
	"ta", //  Tamil
	"ta_IN", //  Tamil (India)
	"ta_LK", //  Tamil (Sri Lanka)
	"tcy_IN", //  Tulu (India)
	"te", //  Telugu
	"te_IN", //  Telugu (India)
	"tg_TJ", //  Tajik (Tajikistan)
	"the_NP", //  Chitwania Tharu (Nepal)
	"th", //  Thai
	"th_TH", //  Thai (Thailand)
	"ti", //  Tigrinya
	"ti_ER", //  Tigrinya (Eritrea)
	"ti_ET", //  Tigrinya (Ethiopia)
	"tig_ER", //  Tigre (Eritrea)
	"tk_TM", //  Turkmen (Turkmenistan)
	"tl_PH", //  Tagalog (Philippines)
	"tn_ZA", //  Tswana (South Africa)
	"tr", //  Turkish
	"tr_CY", //  Turkish (Cyprus)
	"tr_TR", //  Turkish (Turkey)
	"ts_ZA", //  Tsonga (South Africa)
	"tt", //  Tatar
	"tt_RU", //  Tatar (Russia)
	"tzm", // Central Atlas Tamazight
	"tzm_MA", // Central Atlas Tamazight (Marrocos)
	"ug_CN", //  Uighur (China)
	"uk", //  Ukrainian
	"uk_UA", //  Ukrainian (Ukraine)
	"unm_US", //  Unami (United States)
	"ur", //  Urdu
	"ur_IN", //  Urdu (India)
	"ur_PK", //  Urdu (Pakistan)
	"uz", //  Uzbek
	"uz_UZ", //  Uzbek (Uzbekistan)
	"ve_ZA", //  Venda (South Africa)
	"vi", //  Vietnamese
	"vi_VN", //  Vietnamese (Vietnam)
	"wa_BE", //  Walloon (Belgium)
	"wae_CH", //  Walser (Switzerland)
	"wal_ET", //  Wolaytta (Ethiopia)
	"wo_SN", //  Wolof (Senegal)
	"xh_ZA", //  Xhosa (South Africa)
	"yi_US", //  Yiddish (United States)
	"yo_NG", //  Yoruba (Nigeria)
	"yue_HK", //  Yue Chinese (Hong Kong)
	"zh", //  Chinese
	"zh_CN", //  Chinese (China)
	"zh_HK", //  Chinese (Hong Kong)
	"zh_SG", //  Chinese (Singapore)
	"zh_TW", //  Chinese (Taiwan)
	"zu_ZA", //  Zulu (South Africa)
	0
};

static const char *locale_names[] = {
	"Afar",
	"Afar (Djibouti)",
	"Afar (Eritrea)",
	"Afar (Ethiopia)",
	"Afrikaans",
	"Afrikaans (South Africa)",
	"Aguaruna (Peru)",
	"Akan (Ghana)",
	"Amharic (Ethiopia)",
	"Aragonese (Spain)",
	"Angika (India)",
	"Arabic",
	"Arabic (United Arab Emirates)",
	"Arabic (Bahrain)",
	"Arabic (Algeria)",
	"Arabic (Egypt)",
	"Arabic (India)",
	"Arabic (Iraq)",
	"Arabic (Jordan)",
	"Arabic (Kuwait)",
	"Arabic (Lebanon)",
	"Arabic (Libya)",
	"Arabic (Morocco)",
	"Arabic (Oman)",
	"Arabic (Qatar)",
	"Arabic (Saudi Arabia)",
	"Arabic (Sudan)",
	"Arabic (South Soudan)",
	"Arabic (Syria)",
	"Arabic (Tunisia)",
	"Arabic (Yemen)",
	"Assamese (India)",
	"Asturian (Spain)",
	"Southern Aymara (Peru)",
	"Aymara (Peru)",
	"Azerbaijani",
	"Azerbaijani (Azerbaijan)",
	"Belarusian",
	"Belarusian (Belarus)",
	"Bemba (Zambia)",
	"Berber languages (Algeria)",
	"Berber languages (Morocco)",
	"Bulgarian",
	"Bulgarian (Bulgaria)",
	"Bhili (India)",
	"Bhojpuri (India)",
	"Bislama (Tuvalu)",
	"Bengali",
	"Bengali (Bangladesh)",
	"Bengali (India)",
	"Tibetan",
	"Tibetan (China)",
	"Tibetan (India)",
	"Breton",
	"Breton (France)",
	"Bodo (India)",
	"Bosnian (Bosnia and Herzegovina)",
	"Bilin (Eritrea)",
	"Catalan",
	"Catalan (Andorra)",
	"Catalan (Spain)",
	"Catalan (France)",
	"Catalan (Italy)",
	"Chechen (Russia)",
	"Cherokee (United States)",
	"Mandarin Chinese (Taiwan)",
	"Crimean Tatar (Ukraine)",
	"Kashubian (Poland)",
	"Czech",
	"Czech (Czech Republic)",
	"Chuvash (Russia)",
	"Welsh (United Kingdom)",
	"Danish",
	"Danish (Denmark)",
	"German",
	"German (Austria)",
	"German (Belgium)",
	"German (Switzerland)",
	"German (Germany)",
	"German (Italy)",
	"German (Luxembourg)",
	"Dogri (India)",
	"Dhivehi (Maldives)",
	"Dzongkha (Bhutan)",
	"Greek",
	"Greek (Cyprus)",
	"Greek (Greece)",
	"English",
	"English (Antigua and Barbuda)",
	"English (Australia)",
	"English (Botswana)",
	"English (Canada)",
	"English (Denmark)",
	"English (United Kingdom)",
	"English (Hong Kong)",
	"English (Ireland)",
	"English (Israel)",
	"English (India)",
	"English (Nigeria)",
	"English (New Zealand)",
	"English (Philippines)",
	"English (Singapore)",
	"English (United States)",
	"English (South Africa)",
	"English (Zambia)",
	"English (Zimbabwe)",
	"Esperanto",
	"Spanish",
	"Spanish (Argentina)",
	"Spanish (Bolivia)",
	"Spanish (Chile)",
	"Spanish (Colombia)",
	"Spanish (Costa Rica)",
	"Spanish (Cuba)",
	"Spanish (Dominican Republic)",
	"Spanish (Ecuador)",
	"Spanish (Spain)",
	"Spanish (Guatemala)",
	"Spanish (Honduras)",
	"Spanish (Mexico)",
	"Spanish (Nicaragua)",
	"Spanish (Panama)",
	"Spanish (Peru)",
	"Spanish (Puerto Rico)",
	"Spanish (Paraguay)",
	"Spanish (El Salvador)",
	"Spanish (United States)",
	"Spanish (Uruguay)",
	"Spanish (Venezuela)",
	"Estonian",
	"Estonian (Estonia)",
	"Basque",
	"Basque (Spain)",
	"Persian",
	"Persian (Iran)",
	"Fulah (Senegal)",
	"Finnish",
	"Finnish (Finland)",
	"Filipino",
	"Filipino (Philippines)",
	"Faroese (Faroe Islands)",
	"French",
	"French (Belgium)",
	"French (Canada)",
	"French (Switzerland)",
	"French (France)",
	"French (Luxembourg)",
	"Friulian (Italy)",
	"Western Frisian (Germany)",
	"Western Frisian (Netherlands)",
	"Irish",
	"Irish (Ireland)",
	"Scottish Gaelic (United Kingdom)",
	"Geez (Eritrea)",
	"Geez (Ethiopia)",
	"Galician",
	"Galician (Spain)",
	"Gujarati (India)",
	"Manx (United Kingdom)",
	"Hakka Chinese (Taiwan)",
	"Hausa (Nigeria)",
	"Hebrew",
	"Hebrew (Israel)",
	"Hindi",
	"Hindi (India)",
	"Chhattisgarhi (India)",
	"Croatian",
	"Croatian (Croatia)",
	"Upper Sorbian (Germany)",
	"Haitian (Haiti)",
	"Hungarian",
	"Hungarian (Hungary)",
	"Huastec (Mexico)",
	"Armenian (Armenia)",
	"Interlingua (France)",
	"Indonesian",
	"Indonesian (Indonesia)",
	"Igbo (Nigeria)",
	"Inupiaq (Canada)",
	"Icelandic",
	"Icelandic (Iceland)",
	"Italian",
	"Italian (Switzerland)",
	"Italian (Italy)",
	"Inuktitut (Canada)",
	"Japanese",
	"Japanese (Japan)",
	"Kabyle (Algeria)",
	"Georgian",
	"Georgian (Georgia)",
	"Kazakh (Kazakhstan)",
	"Kalaallisut (Greenland)",
	"Central Khmer",
	"Central Khmer (Cambodia)",
	"Kannada (India)",
	"Konkani (India)",
	"Korean",
	"Korean (South Korea)",
	"Kashmiri (India)",
	"Kurdish",
	"Kurdish (Turkey)",
	"Cornish (United Kingdom)",
	"Kirghiz (Kyrgyzstan)",
	"Luxembourgish (Luxembourg)",
	"Ganda (Uganda)",
	"Limburgan (Belgium)",
	"Limburgan (Netherlands)",
	"Ligurian (Italy)",
	"Lingala (Congo)",
	"Lao (Laos)",
	"Lithuanian",
	"Lithuanian (Lithuania)",
	"Latvian",
	"Latvian (Latvia)",
	"Literary Chinese (Taiwan)",
	"Magahi (India)",
	"Maithili (India)",
	"Malagasy (Madagascar)",
	"Marshallese (Marshall Islands)",
	"Eastern Mari (Russia)",
	"Māori",
	"Māori (New Zealand)",
	"Mískito (Nicaragua)",
	"Macedonian",
	"Macedonian (Macedonia)",
	"Malayalam",
	"Malayalam (India)",
	"Manipuri (India)",
	"Mongolian (Mongolia)",
	"Marathi",
	"Marathi (India)",
	"Malay",
	"Malay (Malaysia)",
	"Maltese",
	"Maltese (Malta)",
	"Burmese (Myanmar)",
	"Erzya (Russia)",
	"Nahuatl languages (Mexico)",
	"Min Nan Chinese (Taiwan)",
	"Norwegian Bokmål",
	"Norwegian Bokmål (Norway)",
	"Low German (Germany)",
	"Low German (Netherlands)",
	"Nepali (Nepal)",
	"Central Nahuatl (Mexico)",
	"Niuean (Niue)",
	"Niuean (New Zealand)",
	"Dutch",
	"Dutch (Aruba)",
	"Dutch (Belgium)",
	"Dutch (Netherlands)",
	"Norwegian Nynorsk",
	"Norwegian Nynorsk (Norway)",
	"South Ndebele (South Africa)",
	"Pedi (South Africa)",
	"Occitan (France)",
	"Oromo",
	"Oromo (Ethiopia)",
	"Oromo (Kenya)",
	"Oriya",
	"Oriya (India)",
	"Ossetian (Russia)",
	"Panjabi (India)",
	"Papiamento",
	"Papiamento (Netherlands Antilles)",
	"Papiamento (Aruba)",
	"Papiamento (Curaçao)",
	"Panjabi (Pakistan)",
	"Polish",
	"Polish (Poland)",
	"Pirate",
	"Pushto (Afghanistan)",
	"Portuguese",
	"Portuguese (Brazil)",
	"Portuguese (Portugal)",
	"Ayacucho Quechua (Peru)",
	"Cusco Quechua (Peru)",
	"Rajasthani (India)",
	"Romanian",
	"Romanian (Romania)",
	"Russian",
	"Russian (Russia)",
	"Russian (Ukraine)",
	"Kinyarwanda (Rwanda)",
	"Sanskrit (India)",
	"Santali (India)",
	"Sardinian (Italy)",
	"Scots (Scotland)",
	"Sindhi (India)",
	"Northern Sami (Norway)",
	"Samogitian (Lithuania)",
	"Shuswap (Canada)",
	"Sidamo (Ethiopia)",
	"Sinhala",
	"Sinhala (Sri Lanka)",
	"Slovak",
	"Slovak (Slovakia)",
	"Slovenian",
	"Slovenian (Slovenia)",
	"Somali",
	"Somali (Djibouti)",
	"Somali (Ethiopia)",
	"Somali (Kenya)",
	"Somali (Somalia)",
	"Songhai languages (Mali)",
	"Albanian",
	"Albanian (Albania)",
	"Albanian (Kosovo)",
	"Albanian (Macedonia)",
	"Serbian",
	"Serbian (Cyrillic)",
	"Serbian (Latin)",
	"Serbian (Montenegro)",
	"Serbian (Serbia)",
	"Swati (South Africa)",
	"Southern Sotho (South Africa)",
	"Swedish",
	"Swedish (Finland)",
	"Swedish (Sweden)",
	"Swahili (Kenya)",
	"Swahili (Tanzania)",
	"Silesian (Poland)",
	"Tamil",
	"Tamil (India)",
	"Tamil (Sri Lanka)",
	"Tulu (India)",
	"Telugu",
	"Telugu (India)",
	"Tajik (Tajikistan)",
	"Chitwania Tharu (Nepal)",
	"Thai",
	"Thai (Thailand)",
	"Tigrinya",
	"Tigrinya (Eritrea)",
	"Tigrinya (Ethiopia)",
	"Tigre (Eritrea)",
	"Turkmen (Turkmenistan)",
	"Tagalog (Philippines)",
	"Tswana (South Africa)",
	"Turkish",
	"Turkish (Cyprus)",
	"Turkish (Turkey)",
	"Tsonga (South Africa)",
	"Tatar",
	"Tatar (Russia)",
	"Central Atlas Tamazight",
	"Central Atlas Tamazight (Marrocos)",
	"Uighur (China)",
	"Ukrainian",
	"Ukrainian (Ukraine)",
	"Unami (United States)",
	"Urdu",
	"Urdu (India)",
	"Urdu (Pakistan)",
	"Uzbek",
	"Uzbek (Uzbekistan)",
	"Venda (South Africa)",
	"Vietnamese",
	"Vietnamese (Vietnam)",
	"Walloon (Belgium)",
	"Walser (Switzerland)",
	"Wolaytta (Ethiopia)",
	"Wolof (Senegal)",
	"Xhosa (South Africa)",
	"Yiddish (United States)",
	"Yoruba (Nigeria)",
	"Yue Chinese (Hong Kong)",
	"Chinese",
	"Chinese (China)",
	"Chinese (Hong Kong)",
	"Chinese (Singapore)",
	"Chinese (Taiwan)",
	"Zulu (South Africa)",
	0
};

// Windows has some weird locale identifiers which do not honor the ISO 639-1
// standardized nomenclature. Whenever those don't conflict with existing ISO
// identifiers, we override them.
//
// Reference:
// - https://msdn.microsoft.com/en-us/library/windows/desktop/ms693062(v=vs.85).aspx

static const char *locale_renames[][2] = {
	{ "in", "id" }, //  Indonesian
	{ "iw", "he" }, //  Hebrew
	{ "no", "nb" }, //  Norwegian Bokmål
	{ NULL, NULL }
};

///////////////////////////////////////////////

PoolVector<String> Translation::_get_messages() const {

	PoolVector<String> msgs;
	msgs.resize(translation_map.size() * 2);
	int idx = 0;
	for (const Map<StringName, StringName>::Element *E = translation_map.front(); E; E = E->next()) {

		msgs.set(idx + 0, E->key());
		msgs.set(idx + 1, E->get());
		idx += 2;
	}

	return msgs;
}

PoolVector<String> Translation::_get_message_list() const {

	PoolVector<String> msgs;
	msgs.resize(translation_map.size());
	int idx = 0;
	for (const Map<StringName, StringName>::Element *E = translation_map.front(); E; E = E->next()) {

		msgs.set(idx, E->key());
		idx += 1;
	}

	return msgs;
}

void Translation::_set_messages(const PoolVector<String> &p_messages) {

	int msg_count = p_messages.size();
	ERR_FAIL_COND(msg_count % 2);

	PoolVector<String>::Read r = p_messages.read();

	for (int i = 0; i < msg_count; i += 2) {

		add_message(r[i + 0], r[i + 1]);
	}
}

void Translation::set_locale(const String &p_locale) {

	String univ_locale = TranslationServer::standardize_locale(p_locale);

	if (!TranslationServer::is_locale_valid(univ_locale)) {
		String trimmed_locale = TranslationServer::get_language_code(univ_locale);

		ERR_FAIL_COND_MSG(!TranslationServer::is_locale_valid(trimmed_locale), "Invalid locale: " + trimmed_locale + ".");

		locale = trimmed_locale;
	} else {
		locale = univ_locale;
	}

	if (OS::get_singleton()->get_main_loop()) {
		OS::get_singleton()->get_main_loop()->notification(MainLoop::NOTIFICATION_TRANSLATION_CHANGED);
	}
}

void Translation::add_message(const StringName &p_src_text, const StringName &p_xlated_text) {

	translation_map[p_src_text] = p_xlated_text;
}
StringName Translation::get_message(const StringName &p_src_text) const {

	const Map<StringName, StringName>::Element *E = translation_map.find(p_src_text);
	if (!E)
		return StringName();

	return E->get();
}

void Translation::erase_message(const StringName &p_src_text) {

	translation_map.erase(p_src_text);
}

void Translation::get_message_list(List<StringName> *r_messages) const {

	for (const Map<StringName, StringName>::Element *E = translation_map.front(); E; E = E->next()) {

		r_messages->push_back(E->key());
	}
}

int Translation::get_message_count() const {

	return translation_map.size();
};

void Translation::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_locale", "locale"), &Translation::set_locale);
	ClassDB::bind_method(D_METHOD("get_locale"), &Translation::get_locale);
	ClassDB::bind_method(D_METHOD("add_message", "src_message", "xlated_message"), &Translation::add_message);
	ClassDB::bind_method(D_METHOD("get_message", "src_message"), &Translation::get_message);
	ClassDB::bind_method(D_METHOD("erase_message", "src_message"), &Translation::erase_message);
	ClassDB::bind_method(D_METHOD("get_message_list"), &Translation::_get_message_list);
	ClassDB::bind_method(D_METHOD("get_message_count"), &Translation::get_message_count);
	ClassDB::bind_method(D_METHOD("_set_messages"), &Translation::_set_messages);
	ClassDB::bind_method(D_METHOD("_get_messages"), &Translation::_get_messages);

	ADD_PROPERTY(PropertyInfo(Variant::POOL_STRING_ARRAY, "messages", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "_set_messages", "_get_messages");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "locale"), "set_locale", "get_locale");
}

Translation::Translation() :
		locale("en") {
}

///////////////////////////////////////////////

bool TranslationServer::is_locale_valid(const String &p_locale) {

	const char **ptr = locale_list;

	while (*ptr) {

		if (*ptr == p_locale)
			return true;
		ptr++;
	}

	return false;
}

String TranslationServer::standardize_locale(const String &p_locale) {

	// Replaces '-' with '_' for macOS Sierra-style locales
	String univ_locale = p_locale.replace("-", "_");

	// Handles known non-ISO locale names used e.g. on Windows
	int idx = 0;
	while (locale_renames[idx][0] != NULL) {
		if (locale_renames[idx][0] == univ_locale) {
			univ_locale = locale_renames[idx][1];
			break;
		}
		idx++;
	}

	return univ_locale;
}

String TranslationServer::get_language_code(const String &p_locale) {

	ERR_FAIL_COND_V_MSG(p_locale.length() < 2, p_locale, "Invalid locale '" + p_locale + "'.");
	// Most language codes are two letters, but some are three,
	// so we have to look for a regional code separator ('_' or '-')
	// to extract the left part.
	// For example we get 'nah_MX' as input and should return 'nah'.
	int split = p_locale.find("_");
	if (split == -1) {
		split = p_locale.find("-");
	}
	if (split == -1) { // No separator, so the locale is already only a language code.
		return p_locale;
	}
	return p_locale.left(split);
}

void TranslationServer::set_locale(const String &p_locale) {

	String univ_locale = standardize_locale(p_locale);

	if (!is_locale_valid(univ_locale)) {
		String trimmed_locale = get_language_code(univ_locale);
		print_verbose(vformat("Unsupported locale '%s', falling back to '%s'.", p_locale, trimmed_locale));

		if (!is_locale_valid(trimmed_locale)) {
			ERR_PRINTS(vformat("Unsupported locale '%s', falling back to 'en'.", trimmed_locale));
			locale = "en";
		} else {
			locale = trimmed_locale;
		}
	} else {
		locale = univ_locale;
	}

	if (OS::get_singleton()->get_main_loop()) {
		OS::get_singleton()->get_main_loop()->notification(MainLoop::NOTIFICATION_TRANSLATION_CHANGED);
	}

	ResourceLoader::reload_translation_remaps();
}

String TranslationServer::get_locale() const {

	return locale;
}

String TranslationServer::get_locale_name(const String &p_locale) const {

	if (!locale_name_map.has(p_locale)) return String();
	return locale_name_map[p_locale];
}

Array TranslationServer::get_loaded_locales() const {
	Array locales;
	for (const Set<Ref<Translation> >::Element *E = translations.front(); E; E = E->next()) {

		const Ref<Translation> &t = E->get();
		ERR_FAIL_COND_V(t.is_null(), Array());
		String l = t->get_locale();

		locales.push_back(l);
	}

	return locales;
}

Vector<String> TranslationServer::get_all_locales() {

	Vector<String> locales;

	const char **ptr = locale_list;

	while (*ptr) {
		locales.push_back(*ptr);
		ptr++;
	}

	return locales;
}

Vector<String> TranslationServer::get_all_locale_names() {

	Vector<String> locales;

	const char **ptr = locale_names;

	while (*ptr) {
		locales.push_back(String::utf8(*ptr));
		ptr++;
	}

	return locales;
}

void TranslationServer::add_translation(const Ref<Translation> &p_translation) {

	translations.insert(p_translation);
}
void TranslationServer::remove_translation(const Ref<Translation> &p_translation) {

	translations.erase(p_translation);
}

void TranslationServer::clear() {

	translations.clear();
};

StringName TranslationServer::translate(const StringName &p_message) const {

	// Match given message against the translation catalog for the project locale.

	if (!enabled)
		return p_message;

	ERR_FAIL_COND_V_MSG(locale.length() < 2, p_message, "Could not translate message as configured locale '" + locale + "' is invalid.");

	// Locale can be of the form 'll_CC', i.e. language code and regional code,
	// e.g. 'en_US', 'en_GB', etc. It might also be simply 'll', e.g. 'en'.
	// To find the relevant translation, we look for those with locale starting
	// with the language code, and then if any is an exact match for the long
	// form. If not found, we fall back to a near match (another locale with
	// same language code).

	// Note: ResourceLoader::_path_remap reproduces this locale near matching
	// logic, so be sure to propagate changes there when changing things here.

	StringName res;
	String lang = get_language_code(locale);
	bool near_match = false;

	for (const Set<Ref<Translation> >::Element *E = translations.front(); E; E = E->next()) {
		const Ref<Translation> &t = E->get();
		ERR_FAIL_COND_V(t.is_null(), p_message);
		String l = t->get_locale();

		bool exact_match = (l == locale);
		if (!exact_match) {
			if (near_match) {
				continue; // Only near-match once, but keep looking for exact matches.
			}
			if (get_language_code(l) != lang) {
				continue; // Language code does not match.
			}
		}

		StringName r = t->get_message(p_message);
		if (!r) {
			continue;
		}
		res = r;

		if (exact_match) {
			break;
		} else {
			near_match = true;
		}
	}

	if (!res && fallback.length() >= 2) {
		// Try again with the fallback locale.
		String fallback_lang = get_language_code(fallback);
		near_match = false;

		for (const Set<Ref<Translation> >::Element *E = translations.front(); E; E = E->next()) {
			const Ref<Translation> &t = E->get();
			ERR_FAIL_COND_V(t.is_null(), p_message);
			String l = t->get_locale();

			bool exact_match = (l == fallback);
			if (!exact_match) {
				if (near_match) {
					continue; // Only near-match once, but keep looking for exact matches.
				}
				if (get_language_code(l) != fallback_lang) {
					continue; // Language code does not match.
				}
			}

			StringName r = t->get_message(p_message);
			if (!r) {
				continue;
			}
			res = r;

			if (exact_match) {
				break;
			} else {
				near_match = true;
			}
		}
	}

	if (!res) {
		return p_message;
	}

	return res;
}

TranslationServer *TranslationServer::singleton = NULL;

bool TranslationServer::_load_translations(const String &p_from) {

	if (ProjectSettings::get_singleton()->has_setting(p_from)) {
		PoolVector<String> translations = ProjectSettings::get_singleton()->get(p_from);

		int tcount = translations.size();

		if (tcount) {
			PoolVector<String>::Read r = translations.read();

			for (int i = 0; i < tcount; i++) {

				Ref<Translation> tr = ResourceLoader::load(r[i]);
				if (tr.is_valid())
					add_translation(tr);
			}
		}
		return true;
	}

	return false;
}

void TranslationServer::setup() {

	String test = GLOBAL_DEF("locale/test", "");
	test = test.strip_edges();
	if (test != "")
		set_locale(test);
	else
		set_locale(OS::get_singleton()->get_locale());
	fallback = GLOBAL_DEF("locale/fallback", "en");
#ifdef TOOLS_ENABLED

	{
		String options = "";
		int idx = 0;
		while (locale_list[idx]) {
			if (idx > 0)
				options += ",";
			options += locale_list[idx];
			idx++;
		}
		ProjectSettings::get_singleton()->set_custom_property_info("locale/fallback", PropertyInfo(Variant::STRING, "locale/fallback", PROPERTY_HINT_ENUM, options));
	}
#endif
	//load translations
}

void TranslationServer::set_tool_translation(const Ref<Translation> &p_translation) {
	tool_translation = p_translation;
}

StringName TranslationServer::tool_translate(const StringName &p_message) const {

	if (tool_translation.is_valid()) {
		StringName r = tool_translation->get_message(p_message);

		if (r) {
			return r;
		}
	}

	return p_message;
}

void TranslationServer::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_locale", "locale"), &TranslationServer::set_locale);
	ClassDB::bind_method(D_METHOD("get_locale"), &TranslationServer::get_locale);

	ClassDB::bind_method(D_METHOD("get_locale_name", "locale"), &TranslationServer::get_locale_name);

	ClassDB::bind_method(D_METHOD("translate", "message"), &TranslationServer::translate);

	ClassDB::bind_method(D_METHOD("add_translation", "translation"), &TranslationServer::add_translation);
	ClassDB::bind_method(D_METHOD("remove_translation", "translation"), &TranslationServer::remove_translation);

	ClassDB::bind_method(D_METHOD("clear"), &TranslationServer::clear);

	ClassDB::bind_method(D_METHOD("get_loaded_locales"), &TranslationServer::get_loaded_locales);
}

void TranslationServer::load_translations() {

	String locale = get_locale();
	_load_translations("locale/translations"); //all
	_load_translations("locale/translations_" + locale.substr(0, 2));

	if (locale.substr(0, 2) != locale) {
		_load_translations("locale/translations_" + locale);
	}
}

TranslationServer::TranslationServer() :
		locale("en"),
		enabled(true) {
	singleton = this;

	for (int i = 0; locale_list[i]; ++i) {

		locale_name_map.insert(locale_list[i], String::utf8(locale_names[i]));
	}
}
