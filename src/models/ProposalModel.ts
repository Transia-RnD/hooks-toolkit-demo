import {
  BaseModel,
  Metadata,
  UInt8,
  UInt32,
  XFL,
  Currency,
  XRPAddress,
} from '@transia/hooks-toolkit/dist/npm/src/libs/binary-models'

export class ProposalModel extends BaseModel {
  status: UInt8 // 0 open 1 closed
  expiration: UInt32 // Last Ledger Sequence
  orderType: UInt8 // 0 BUY 1 SELL
  price: XFL // Price as XFL
  quantity: XFL // Quantity as XFL
  currency: Currency // Currency
  issuer: XRPAddress // Issuer

  constructor(
    status: UInt32,
    expiration: UInt32,
    orderType: UInt8,
    price: XFL,
    quantity: XFL,
    currency: Currency,
    issuer: XRPAddress
  ) {
    super()
    this.status = status
    this.expiration = expiration
    this.orderType = orderType
    this.price = price
    this.quantity = quantity
    this.currency = currency
    this.issuer = issuer
  }

  getMetadata(): Metadata {
    return [
      { field: 'status', type: 'uint8' },
      { field: 'expiration', type: 'uint32' },
      { field: 'orderType', type: 'uint8' },
      { field: 'price', type: 'xfl' },
      { field: 'quantity', type: 'xfl' },
      { field: 'currency', type: 'currency' },
      { field: 'issuer', type: 'xrpAddress' },
    ]
  }

  toJSON() {
    return {
      status: this.status,
      expiration: this.expiration,
      orderType: this.orderType,
      price: this.price,
      quantity: this.quantity,
      currency: this.currency,
      issuer: this.issuer,
    }
  }
}