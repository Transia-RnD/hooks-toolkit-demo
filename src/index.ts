import {
  Xrpld,
  createHookPayload,
  iHookParamEntry,
  iHookParamName,
  iHookParamValue,
  setHooksV3,
  SetHookParams,
  ExecutionUtility,
  StateUtility,
  floatToLEXfl,
} from '@transia/hooks-toolkit'
// NOT EXPORTED
import {
  XrplIntegrationTestContext,
  close,
  serverUrl,
  setupClient,
} from '@transia/hooks-toolkit/dist/npm/src/libs/xrpl-helpers'
import {
  URITokenMint,
  URITokenCreateSellOffer,
  Payment,
  TrustSet,
  Invoke,
  SetHookFlags,
  TransactionMetadata,
  convertStringToHex,
} from '@transia/xrpl'
import { IssuedCurrencyAmount } from '@transia/xrpl/dist/npm/models/common'
import { AccountID } from '@transia/ripple-binary-codec/dist/types'
import { sign } from '@transia/ripple-keypairs'
import { hashURIToken } from '@transia/xrpl/dist/npm/utils/hashes'
import { ProposalModelV2 } from './models/ProposalModelV2'
import {
  decodeModel,
  hexToXfl,
} from '@transia/hooks-toolkit/dist/npm/src/libs/binary-models'
import { ICModel } from './models/ICModel'

export async function setHooks(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hookWallet = testContext.alice
  const fmWallet = testContext.bob

  const txAmount1: IssuedCurrencyAmount = {
    value: '100000000',
    currency: 'USD',
    issuer: testContext.gw.classicAddress,
  }
  const builtTx1: TrustSet = {
    TransactionType: 'TrustSet',
    Account: hookWallet.classicAddress,
    LimitAmount: txAmount1,
  }
  await Xrpld.submit(testContext.client, {
    wallet: hookWallet,
    tx: builtTx1,
  })

  // SET HOOK IN - 3MM
  const fmAcctHex = AccountID.from(fmWallet.classicAddress).toHex()
  const hook1Param1 = new iHookParamEntry(
    new iHookParamName('IMC'),
    new iHookParamValue('01', true)
  )
  const hook1Param2 = new iHookParamEntry(
    new iHookParamName('495300', true), // IS
    new iHookParamValue(fmAcctHex, true)
  )
  const hook1 = createHookPayload(
    0,
    '3mm',
    'nfo',
    SetHookFlags.hsfOverride,
    ['Invoke'],
    [hook1Param1.toXrpl(), hook1Param2.toXrpl()]
  )

  // SET HOOK IN - NAV
  const hook2Param1 = new iHookParamEntry(
    new iHookParamName('PK'),
    new iHookParamValue(fmWallet.publicKey, true)
  )
  const hook2 = createHookPayload(
    0,
    'nav',
    'nfo',
    SetHookFlags.hsfOverride,
    ['Invoke'],
    [hook2Param1.toXrpl()]
  )

  // SET HOOK IN - NFO
  const hook3 = createHookPayload(0, 'nfo', 'nfo', SetHookFlags.hsfOverride, [
    'Payment',
  ])

  await setHooksV3({
    client: testContext.client,
    seed: hookWallet.seed,
    hooks: [{ Hook: hook1 }, { Hook: hook2 }, { Hook: hook3 }],
  } as SetHookParams)

  await testContext.client.disconnect()
}

export async function setNAV(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hookWallet = testContext.alice
  const fmWallet = testContext.bob

  // INVOKE IN
  const nav = String(1)
  const tx1Param1 = new iHookParamEntry(
    new iHookParamName('NAV'),
    new iHookParamValue(floatToLEXfl(nav), true)
  )
  const tx1Param2 = new iHookParamEntry(
    new iHookParamName('SIG'),
    new iHookParamValue(sign(floatToLEXfl(nav), fmWallet.privateKey), true)
  )

  const builtTx: Invoke = {
    TransactionType: 'Invoke',
    Account: fmWallet.classicAddress,
    Destination: hookWallet.classicAddress,
    HookParameters: [tx1Param1.toXrpl(), tx1Param2.toXrpl()],
  }
  await Xrpld.submit(testContext.client, {
    wallet: fmWallet,
    tx: builtTx,
  })
  const state = (await StateUtility.getHookStateDir(
    testContext.client,
    hookWallet.classicAddress,
    'nfo'
  )) as any[]
  console.log(state)
  console.log(hexToXfl(state[0].HookStateData))
  await testContext.client.disconnect()
}

export async function buyNFO(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hookWallet = testContext.alice
  const client1Wallet = testContext.elsa

  // TRUST SET IN
  const txAmount1: IssuedCurrencyAmount = {
    value: '100000000',
    currency: 'NAV',
    issuer: hookWallet.classicAddress,
  }
  const builtTx1: TrustSet = {
    TransactionType: 'TrustSet',
    Account: client1Wallet.classicAddress,
    LimitAmount: txAmount1,
  }
  await Xrpld.submit(testContext.client, {
    wallet: client1Wallet,
    tx: builtTx1,
  })

  // PAYMENT IN
  const txAmount2: IssuedCurrencyAmount = {
    value: '100',
    currency: testContext.ic.currency as string,
    issuer: testContext.ic.issuer as string,
  }

  const builtTx2: Payment = {
    TransactionType: 'Payment',
    Account: client1Wallet.classicAddress,
    Destination: hookWallet.classicAddress,
    Amount: txAmount2,
  }
  const result2 = await Xrpld.submit(testContext.client, {
    wallet: client1Wallet,
    tx: builtTx2,
  })
  const result2Executions = await ExecutionUtility.getHookExecutionsFromMeta(
    testContext.client,
    result2.meta as TransactionMetadata
  )
  console.log(result2Executions.executions[0].HookReturnString)
  await close(testContext.client)
  await testContext.client.disconnect()
}

export async function setUp3mm(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const hookWallet = testContext.alice
  // const fmWallet = testContext.bob

  // INVOKE OUT
  const builtTx: Invoke = {
    TransactionType: 'Invoke',
    Account: hookWallet.classicAddress,
  }
  const result = await Xrpld.submit(testContext.client, {
    wallet: hookWallet,
    tx: builtTx,
  })
  const hookExecutions = await ExecutionUtility.getHookExecutionsFromMeta(
    testContext.client,
    result.meta as TransactionMetadata
  )
  console.log(hookExecutions.executions[0].HookReturnString)
  const state = await StateUtility.getHookStateDir(
    testContext.client,
    hookWallet.classicAddress,
    'nfo'
  )
  console.log(state)
  await close(testContext.client)
  await testContext.client.disconnect()
}

export async function buyURIToken(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const gwWallet = testContext.gw
  const hookWallet = testContext.alice
  const fmWallet = testContext.bob

  // URITOKEN_MINT && URITOKEN_CREATE_SELL_OFFER IN
  const uri = `ipfs://${Math.random()}`
  const builtTx2: URITokenMint = {
    TransactionType: 'URITokenMint',
    Account: gwWallet.classicAddress,
    URI: convertStringToHex(uri),
  }
  await Xrpld.submit(testContext.client, {
    wallet: gwWallet,
    tx: builtTx2,
  })
  await close(testContext.client)
  const uriTokenID = hashURIToken(gwWallet.classicAddress, uri)
  const tx3Amount: IssuedCurrencyAmount = {
    value: '10',
    currency: 'USD',
    issuer: testContext.gw.classicAddress,
  }
  const builtTx3: URITokenCreateSellOffer = {
    TransactionType: 'URITokenCreateSellOffer',
    Account: gwWallet.classicAddress,
    Amount: tx3Amount,
    URITokenID: uriTokenID,
  }
  await Xrpld.submit(testContext.client, {
    wallet: gwWallet,
    tx: builtTx3,
  })

  // INVOKE IN
  const proposalModelV2 = new ProposalModelV2(
    0, // 0 open 1 closed
    12, // expiration
    0, // 0 buy 1 sell 2 cancel
    1, // quantity
    10, // price
    'USD', // currency
    gwWallet.classicAddress, // address
    uriTokenID // hash id
  )
  console.log(decodeModel(proposalModelV2.encode(), ProposalModelV2))
  console.log(proposalModelV2.encode())
  console.log(proposalModelV2.encode().length / 2)
  console.log(proposalModelV2.encode().padStart(128, '0'))
  const tx4Param1 = new iHookParamEntry(
    new iHookParamName('T'),
    new iHookParamValue('55' + '01', true) // U - 01 (proposal #)
  )

  const tx4Param2 = new iHookParamEntry(
    new iHookParamName('V'),
    new iHookParamValue(proposalModelV2.encode(), true)
  )

  const builtTx4: Invoke = {
    TransactionType: 'Invoke',
    Account: fmWallet.classicAddress,
    Destination: hookWallet.classicAddress,
    HookParameters: [tx4Param1.toXrpl(), tx4Param2.toXrpl()],
  }
  const result4 = await Xrpld.submit(testContext.client, {
    wallet: fmWallet,
    tx: builtTx4,
  })
  const tx4Executions = await ExecutionUtility.getHookExecutionsFromMeta(
    testContext.client,
    result4.meta as TransactionMetadata
  )
  console.log(tx4Executions.executions[0].HookReturnString)

  await close(testContext.client)
  const state = await StateUtility.getHookStateDir(
    testContext.client,
    hookWallet.classicAddress,
    'nfo'
  )
  console.log(state)

  await testContext.client.disconnect()
}

export async function main(): Promise<void> {
  await setHooks()
  // await setNAV()
  // await buyNFO()
  // await setUp3mm()
  // await buyURIToken()
}

// main()

export async function examples(): Promise<void> {
  const icModel = new ICModel(
    10, // price
    'USD', // currency
    'rG1QQv2nh2gr7RCZ1P8YYcBUKCCN633jCn' // address
  )
  console.log(icModel)
  const encoded = icModel.encode()
  console.log(encoded)
  const decodedModel = decodeModel(encoded, ICModel)
  console.log(decodedModel)
}

examples()
